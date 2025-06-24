
//char calibrations[] = "{\"neck\":{\"rotation\":0,\"tiltForward\":0,\"tiltSideways\":0}}";

#ifndef CALIBRATION_H
#define CALIBRATION_H

// This file previously contained a hardcoded 'calibrations' char array.
// With the new LittleFS-based JSON calibration system, this array is no longer needed
// and would cause redefinition errors. It has been removed.
// Calibration data is now loaded from and saved to /calibration.json by the WebServer class.

#endif // CALIBRATION_H
