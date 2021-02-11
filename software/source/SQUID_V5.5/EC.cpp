/*! \EC.cpp
    \Methods for measurement of electric conductivity using
    \Analog Devices famous AD5933 impedance analyzer

     _____    _____    ___    __    ___  _____      _____
    /  _  \  |__    \  \  \  /  \  /  / |__    \   /  _  \
   |  /_\  |  __| _  |  \  \/ __ \/  /   __| _  | |  |_|  |
   |  |____  |   |_| |   \   /  \   /   |   |_| |  \____  |
    \______| |_______|    \_/    \_/    |_______|   _   | |
   Eawag                                           | \__/ |
   Department Urban Water Management                \____/
   Ueberlandstrasse 133
   CH-8600 Duebendorf

   and kofatec GmbH, CH-5735 Pfeffikon, www.kofatec.ch

   The SQUID hardware and software work is licensed under a Creative
   Commons Attribution-ShareAlike 2.0 Generic License and this program is
   free software: you can redistribute it and/or modify it under the terms
   of the GNU Lesser General Public License as published by the Free Software
   Foundation, either version 2.1 of the License, or (at your option) any
   later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Design and Implementation:	Adrian Koller, Christian Ebi
*/
#include "EC.h"
#include "Data.h"
#include "AD5933.h"
#include "Configuration.h"
#include <Streaming.h>

typedef struct
{
  int real;
  int imag;
}  complex_type;


bool initializeEC( Configuration cfg, SquidData& data ) {
  bool success = true;
  // Perform initial configuration. Fail if any one of these fail.
  if (!(AD5933::reset() &&
        AD5933::setInternalClock(true) &&
        AD5933::setStartFrequency(cfg.ecFrequency) &&
        AD5933::setIncrementFrequency(cfg.ecFrequencyIncrement) &&
        AD5933::setNumberIncrements(cfg.ecNumberOfIncrements) &&
        AD5933::setPGAGain(PGA_GAIN_X1) &&
        AD5933::setVoutRange(VOUT_RANGE_1)))
  {
    success = false;
  }
  data.kCell = cfg.ecCalRef * cfg.ecCalRawValue * (1.0 + (cfg.ecCalTalpha / 100.0) * (cfg.ecCalT - 25.0));
  return success;
}


/* do a frequency sweep get AD5933 raw real and imaginary registers */
bool performFrequencySweep(int nFreqSteps, complex_type* complexData) {
  bool success = true;
  int AD5933Real = 0;
  int AD5933Imag = 0;
  int index = 0;

  // **** -->ako, 2021-02-09: see datasheet -> upon init start freq, the user should
  //    allow for some settling time.

  // if placing in the standby or starting the frequency sweep fails, return false
  if (!(AD5933::setPowerMode(POWER_STANDBY) && AD5933::setControlMode(CTRL_INIT_START_FREQ)) )
  {
    success = false;
  }
  // allow for settling
  delay(20);
  // if beginning freq sweep fails, return false
  if ( !AD5933::setControlMode(CTRL_START_FREQ_SWEEP) )
  {
    return false;
  }
  else 
  {
    // Perform the actual sweep, --> timeout if not successful
    unsigned long previous = millis();
    while (((AD5933::readStatusRegister() & STATUS_SWEEP_DONE) != STATUS_SWEEP_DONE && success == true) && ((millis() - previous) <= 3000) && (index < nFreqSteps)) {
      // Get the frequency data for this frequency point
      if (!AD5933::getComplexData(&AD5933Real, &AD5933Imag)) {
        success = false;
      }
      else {
        complexData[index] = {AD5933Real, AD5933Imag}; //write real and imag values to callers variable
        index++;
      }
      AD5933::setControlMode(CTRL_INCREMENT_FREQ);
      if ( millis() < previous )  previous = millis();  //overflow protection
    }
    if (!AD5933::setPowerMode(POWER_STANDBY)) {
      success = false;
    }
  }
  return success;
}


/* obtain a single EC measurement (including autoranging and calibration)*/
bool getEcMeasurement(Configuration cfg, SquidData& data) {
  bool success = true;

  int nFreqSteps = 1;
  complex_type AD5933Data[nFreqSteps] = {0, 0};

  float ecMag = 0.0;
  float ecPhase = 0.0;

  //hard coded AD5933 calibration values for each VOUT_RANGE_X ([VOUT_RANGE_1, VOUT_RANGE_2])
  float gainFactor [] = {0.00000021796, 0.00000047117}; //TODO check number of decimal digits!
  float systemPhase [] = { -1.1128, -1.1072};
  int rangeThreshold = 500; //sets the threshold to 500 ohms -> 2mS

  if (!(AD5933::setVoutRange(VOUT_RANGE_1) &&
        performFrequencySweep(nFreqSteps, AD5933Data))) {
    success = false;
    data.ecRealRaw = 999;
    data.ecImagRaw = 999;
  }
  else {
    ecMag = sqrt(sq((float)AD5933Data->real) + sq((float)AD5933Data->imag)); //TODO check number of decimal digits!

    if (AD5933Data->real == 0) {
      ecPhase = 1.570796; //prevent division by 0, replace phase with pi/2 instead.
    }
    else {
      ecPhase = atan((float)AD5933Data->imag / ((float)AD5933Data->real));
    }

    //Calibration of AD5933 readouts based on the selected VOUT_RANGE_X
    ecMag = 1.0 / (gainFactor[0] * ecMag);
    ecPhase = ecPhase - systemPhase [0];

    data.ecRealRaw = ecMag * cos(ecPhase);
    data.ecImagRaw = ecMag * sin(ecPhase); //unused at this stage

    //repeat measurement if real value is lower than the range threshold!
    if (data.ecRealRaw < rangeThreshold) {
      if (!(AD5933::setVoutRange(VOUT_RANGE_4) &&
            performFrequencySweep(nFreqSteps, AD5933Data))) {
        success = false;
        data.ecRealRaw = 999;
        data.ecImagRaw = 999;
      }
      else {
        ecMag = sqrt(sq((float)AD5933Data->real) + sq((float)AD5933Data->imag));
        if (AD5933Data->real == 0) {
          ecPhase = 1.570796; //prevent division by 0, replace phase with pi/2 instead.
        }
        else {
          ecPhase = atan((float)AD5933Data->imag / ((float)AD5933Data->real));
        }

        //Calibration of AD5933 readouts based on the selected VOUT_RANGE_X
        ecMag = 1.0 / (gainFactor[1] * ecMag);
        ecPhase = ecPhase - systemPhase[1];
        data.ecRealRaw = ecMag * cos(ecPhase);
        data.ecImagRaw = ecMag * sin(ecPhase); //unused at this stage
      }
    }
    //calculate real EC value with Cell Constant and temperature compensation
    //if(data.t)
    data.ec = 1.0 / data.ecRealRaw * data.kCell * 1.0 / (1.0 + (cfg.ecCalTalpha / 100.0) * (data.t - 25.0));

  }
  return success;
}
