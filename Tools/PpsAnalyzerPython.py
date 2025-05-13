#!/usr/bin/env python3

#*****************************************************************************************
# Project:     UniversalPpsAnalyzer
#
# Author:      Martin Burri <info@burrima.ch>, Sven Meier <contact@nettimelogic.com>
#
# License:     Copyright (c) 2020, NetTimeLogic GmbH, Switzerland,
#              All rights reserved.
#
#              THIS PROGRAM IS FREE SOFTWARE: YOU CAN REDISTRIBUTE IT AND/OR MODIFY
#              IT UNDER THE TERMS OF THE GNU LESSER GENERAL PUBLIC LICENSE AS
#              PUBLISHED BY THE FREE SOFTWARE FOUNDATION, VERSION 3.
#
#              THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL, BUT
#              WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
#              MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE. SEE THE GNU
#              LESSER GENERAL LESSER PUBLIC LICENSE FOR MORE DETAILS.
#
#              YOU SHOULD HAVE RECEIVED A COPY OF THE GNU LESSER GENERAL PUBLIC LICENSE
#              ALONG WITH THIS PROGRAM. IF NOT, SEE <http://www.gnu.org/licenses/>.
#
#*****************************************************************************************

from datetime import datetime
from datetime import timedelta
import json
import logging
import matplotlib.pyplot as plot
import os
import random
import serial
import time
import sys
import glob
import signal

logger = logging.getLogger(__name__)
logger.addHandler(logging.NullHandler())


class FpgaAccess():

    """
    Abstract class used by TimeStampingUnit to access TSU registers.
    """

    def readReg(self, addr):
        raise Exception("method pure virtual")

    def writeReg(self, addr, val):
        raise Exception("method pure virtual")


class TimeStampingUnit():

    """
    Class representing a time stamping unit (TSU) instance in the FPGA.

    The PPS Analyzer has 9 time stamping units that can be accessed individually. The TSU is an
    FPGA block that can potentially used in other designs - thus we keep it in an own class.
    """

    def __init__(self, fpgaAccess, baseAddr, instanceIdx):
        """
        Constructor

        @param fpgaAccess  object of type FpgaAccess to access the FPGA
        @param baseAddr  base address of the TSU register set
        @param instanceIdx  index describing the instance 0..x
        """
        self.logger = logger.getChild(f"tsu-{instanceIdx}")
        self.fpga = fpgaAccess
        self.baseAddr = baseAddr
        self._isEnabled = False

    def getVersion(self):
        self.logger.debug("getVersion()")
        rv = self.fpga.readReg(self.baseAddr + 0xC)
        major = (rv & 0xFF000000) >> 24
        minor = (rv & 0x00FF0000) >> 16
        build = (rv & 0x0000FFFF)
        return f"{major}.{minor}.{build}"

    def setPolarity(self, isActiveHigh):
        self.logger.debug(f"setPolarity({isActiveHigh})")
        self.fpga.writeReg(self.baseAddr + 0x08, 1 if isActiveHigh else 0)

    def isPolarityActiveHigh(self):
        self.logger.debug("isPolarityActiveHigh()")
        return (self.fpga.readReg(self.baseAddr + 0x08) > 0)

    def setCableDelay(self, delayNs):
        if delayNs < 0 or delayNs > 0xFFFF:
            raise Exception("delay must be in [0; 0xFFFF]")
        self.logger.debug(f"setCableDelay({delayNs})")
        self.fpga.writeReg(self.baseAddr + 0x20, delayNs)

    def getCableDelay(self):
        self.logger.debug(f"getCableDelay()")
        return self.fpga.readReg(self.baseAddr + 0x20)

    def setEnable(self, isEnabled):
        self.logger.debug(f"setEnable({isEnabled})")
        self.fpga.writeReg(self.baseAddr + 0x00, 1 if isEnabled else 0)
        if isEnabled:
            self.fpga.writeReg(self.baseAddr + 0x34, 1)  # interrupt enable
            self.fpga.writeReg(self.baseAddr + 0x30, 1)  # clear interrupt
        self._isEnabled = isEnabled

    def isEnabled(self, cached=False):
        if cached:
            return self._isEnabled
        else:
            self.logger.debug(f"isEnabled(cached={cached})")
            return (self.fpga.readReg(self.baseAddr + 0x00) > 0)

    def hasTimestamp(self):
        self.logger.debug("hasTimestamp()")
        return (self.fpga.readReg(self.baseAddr + 0x30) > 0)

    def getEventCount(self):
        self.logger.debug("getEventCount()")
        return self.fpga.readReg(self.baseAddr + 0x38)

    def readTimestamp(self):
        self.logger.debug("readTimestamp()")
        ns = self.fpga.readReg(self.baseAddr + 0x44)
        s = self.fpga.readReg(self.baseAddr + 0x48)
        self.fpga.writeReg(self.baseAddr + 0x30, 1)  # clear interrupt
        return s, ns


class PpsAnalyzer(FpgaAccess):

    """
    Class representing the PPS Analyzer. Main class to interact with.
    """

    NUM_TSU = 9

    def __init__(self, port="/dev/ttyUSB1", isHeadless=False):
        """
        Constructor

        @param port  Linux device name of the tty
        @param isHeadless  set to True if no window environment is present
        """
        self.run = True
        signal.signal(signal.SIGTERM, self.abortSignal)
        signal.signal(signal.SIGINT, self.abortSignal)
        signal.signal(signal.SIGABRT, self.abortSignal)
        self.port = port
        self.isHeadless = isHeadless
        if isHeadless:
            plot.switch_backend("Agg")  # be careful with this, read docs!
        self.uart = None
        self.tsu = []
        for i in range(self.NUM_TSU):
            baseAddr = 0x10000000 * (i + 1)
            self.tsu.append(TimeStampingUnit(self, baseAddr, i))

    def __del__(self):
        """
        Destructor
        """
        if self.uart is not None:
            self.uart.close()
            self.uart = None

    def abortSignal(self, signum, frame):
        logger.info("abort logging")
        self.run = False

    def setUp(self, enabledTsus=[]):
        """
        Set up method to be called after creation.

        @param enabledTsus  list of TSUs indexes (0..8) to be enabled
        """
        logger.info(f"setUp(enabledTsus={enabledTsus})")
        self.uart = serial.Serial(port=self.port, baudrate=1000000, timeout=3)
        self.checkConnection()
        logger.info("Connection OK")
        logger.info("Version = " + self.getVersion())
        for i in range(self.NUM_TSU):
            self.tsu[i].setEnable(i in enabledTsus)

    def tearDown(self):
        """
        Clean up method to be called when not used any longer.
        """
        logger.info("tearDown()")
        for i in range(self.NUM_TSU):
            if self.tsu[i].isEnabled(cached=True):
                self.tsu[i].setEnable(False)
        self.uart.close()
        self.uart = None

    def computeChecksum(self, data):
        xor = 0
        for i in range(len(data)):
            xor = xor ^ ord(data[i])
        return format(xor, 'X').zfill(2)

    def communicate(self, code, fields=[], numTries=2):
        """
        Send and receive some data.

        If something goes wrong, a retry is performed. If it fails again, an Exception is thrown.
        The number of total tries (1 + retries) can be passed as parameter. Please keep in mind
        that retries will most probably influence any ongoing measurement recording.

        @param code  command code to be sent (e.g. "WC", "RC", ...)
        @param fields  list of parameters as strings
        @param numTries  number of trials if something goes wrong
        """
        for retry in range(numTries):
            try:
                txData = ','.join(fields)
                txContent = code
                if len(txData) > 0:
                    txContent += "," + txData
                txStr = "$" + txContent + "*" + self.computeChecksum(txContent) + "\r\n"
                txBytes = txStr.encode()
                logger.debug(f"TX: {txBytes}")
                self.uart.write(txBytes)

                rxBytes = self.uart.readline()
                logger.debug(f"RX: {rxBytes}")
                if not rxBytes or rxBytes == b"":
                    raise Exception("No response received")
                rxStr = rxBytes.decode().strip()
                if rxStr[0] != '$':
                    raise Exception("Response does not start with $")
                rxContent, rxChecksum = rxStr[1:].split("*")
                if rxChecksum != self.computeChecksum(rxContent):
                    raise Exception("Wrong response checksum")
                rxFields = rxContent.split(',')
                rxCode = rxFields[0]
                if rxCode == "ER":
                    raise Exception(f"Error code received")
                if rxCode[0] != code[0]:
                    raise Exception(f"Wrong response code")
                return rxFields[1:]

            except Exception as e:
                if retry < (numTries - 1):
                    logger.exception(e)
                    logger.error("RETRYING...")
                else:
                    self.uart.close()
                    raise e

    def readReg(self, addr):
        if type(addr) != str:
            addr = "0x" + format(addr, 'X').zfill(8)
        rv = self.communicate("RC", [addr])
        return int(rv[1], 16)

    def writeReg(self, addr, val):
        if type(addr) != str:
            addr = "0x" + format(addr, 'X').zfill(8)
        if type(val) != str:
            val = "0x" + format(val, 'X').zfill(8)
        self.communicate("WC", [addr, val])

    def checkConnection(self):
        self.communicate("CC", [])

    def getVersion(self):
        return self.tsu[0].getVersion()

    def writeDataToFile(self, data, fileName):
        """
        Writes any given serializable data as json to the given file.
        """
        with open(fileName, 'w') as f:
            json.dump(data, f)

    def readDataFromFile(self, fileName):
        """
        Reads any given json file and re-builds the original data.
        """
        with open(fileName, 'r') as f:
            data = json.load(f)
        return data

    def _getEmptyTimestampRecord(self):
        return {
            "startTime": str(datetime.now()),
            "errors": {
                "numPollingCycleOverruns": 0,
                "numMissedTimeStamps": 0
            },
            "pps-0": [],
            "pps-1": [],
            "pps-2": [],
            "pps-3": [],
            "pps-4": [],
            "pps-5": [],
            "pps-6": [],
            "pps-7": [],
            "pps-8": []
        }

    def recordTimeStamps(self, durationS, fileName=None):
        """
        Records time stamps of all enabled TSUs for the given duration.

        This method returns the recorded data in the following form:
        {
            startTime: '2020-05-21 17:00:33.385372',
            pps-0: [ [ <s>, <ns> ], ...],  # s=seconds, ns=nanoseconds
            pps-1: [ [ <s>, <ns> ], ...],
            ...
            errors: {
                "numMissedTimeStamps": <int>,
                "numPollingCycleOverruns": <int>
            }
        }

        Recording is done via polling. Each TSU is polled twice a second for new time stamps. The
        polling time is based on the host time which can result in a time stamp more more or less
        but is more robust in automated environments. The method will be finished after a
        pre-defined time, even if the board - for any reason - stops working, or if the PPS signal
        is lost.

        If a recoverable error occurs, the recording is continued and the error counted into the
        returned data. Non-recoverable errors throw Exceptions.  Polling cycle overruns indicate
        potential unrecognized time stamp losses. If the overrun is not too much, and the PPS
        signals are stable, the chance for data loss is very low. Counted missed time stamps
        without overruns are an indication for PPS signal glitches. Under normal conditions, all
        error counters should be 0 and it is recommended to assert for this in automated tests.

        The seconds in the returned data are normalized to the start time, meaning that the
        recording log starts at 0.

        If the user provides a fileName, the recorded data structure will be written to that file
        in json format.

        @param durationS  for how long to poll and log in seconds (host time)
        @param fileName  path/name of log file to store data to (if != None)
        """
        logger.info(f"recordTimeStamps({durationS}, {fileName})")

        # get initial event counts and restart stuck TSUs:
        eventCounts = [0, 0, 0, 0, 0, 0, 0, 0, 0]
        for i in range(self.NUM_TSU):
            if self.tsu[i].isEnabled(cached=True):
                eventCounts[i] = self.tsu[i].getEventCount()
                self.tsu[i].readTimestamp()

        # do the polling:
        data = self._getEmptyTimestampRecord()
        s0 = None
        startTime = nextCycleTime = datetime.now()
        while ((datetime.now() - startTime).seconds < durationS) and (self.run == True):
            for tsuIdx in range(self.NUM_TSU):
                tsu = self.tsu[tsuIdx]
                if tsu.isEnabled(cached=True) and tsu.hasTimestamp():
                    # check event count for missed events:
                    count = tsu.getEventCount()
                    if count - eventCounts[tsuIdx] > 1:
                        data["errors"]["numMissedTimeStamps"] += 1
                    eventCounts[tsuIdx] = count

                    # get timestamp and normalize to start point 0:
                    s, ns = tsu.readTimestamp()
                    # compute offset to internal ref:
                    if ns > int(0.5e9):
                        s += 1
                        ns = ns - int(1e9)
                    # normalize to common t0 zero-point in time:
                    if s0 is None:
                        s0 = s
                    s -= s0
                    data[f"pps-{tsuIdx}"].append([s, ns])

            # sleep for remaining cycle time:
            nextCycleTime += timedelta(seconds=0.5)  # time of next cycle
            sleep = (nextCycleTime - datetime.now()).total_seconds()
            if sleep < 0:
                data["errors"]["numPollingCycleOverruns"] += 1
            else:
                time.sleep(sleep)

        if fileName:
            self.writeDataToFile(data, fileName)

        return data

    def recordDummyTimeStamps(self, durationS, fileName=None):
        """
        Creates a dummy timestamp record for unit testing purposes.
        """
        logger.info(f"generateDummyTimestamps({durationS})")
        data = self._getEmptyTimestampRecord()
        for tsuIdx in range(self.NUM_TSU):
            offsetNs = random.randint(-5000, 5000) if tsuIdx > 0 else 0
            for t in range(durationS):
                if self.tsu[tsuIdx].isEnabled(cached=True):
                    jitterNs = random.randint(-50 * (tsuIdx+1), 50 * (tsuIdx+1))
                    s = t
                    ns = offsetNs + jitterNs
                    data[f"pps-{tsuIdx}"].append([s, ns])
        if fileName:
            self.writeDataToFile(data, fileName)
        return data

    def plotTimeStamps(self, data, title=None, ylimits=[None, None], fileName=None):
        """
        Draws and shows a figure out of given recorded time stamp data.

        If isHeadless was set to True in the constructor, the plot is not directly shown (used for
        the case when there is no window environment present or when the method shall end without
        blocking, e.g. in automated test environments). A file name must be provided in this case.

        The user can specify the y-axis limits. The value "None" means "auto".

        The user can also specify it's own figure title.

        @param data  recorded time stamp data to plot
        @param title  figure title (top line in figure)
        @param ylimits  y-axis limits [lower, higher]
        @param fileName  if provided, the plot will be stored to file as well
        """
        logger.info(f"plotTimeStamps(title={title}, ylimits={ylimits}, fileName={fileName})")
        plot.figure(figsize=(15, 10))
        for tsuIdx in range(self.NUM_TSU):
            try:
                numSamples = len(data[f"pps-{tsuIdx}"])
            except Exception:
                continue
            if numSamples == 0:
                continue
            x = []
            y = []
            for i in range(numSamples):
                s, ns = data[f"pps-{tsuIdx}"][i]
                x.append(s)
                y.append(ns)
            if tsuIdx == 0:
                plot.plot(x, y, label="PPS-REF", color="lightgray")
            else:
                plot.plot(x, y, label=f"PPS-{tsuIdx}")

        # axes and labels:
        if title:
            plot.title(title)
        plot.xlabel("time [s]")
        plot.ylabel("offset [ns]")
        plot.axis((None, None, ylimits[0], ylimits[1]))
        plot.legend()

        # enable grid:
        plot.grid(b=True, which='major', color='#666666', linestyle='-')
        plot.minorticks_on()
        plot.grid(b=True, which='minor', color='#999999', linestyle='-', alpha=0.2)

        if fileName:
            plot.savefig(fileName)
        if not self.isHeadless:
            plot.show()
        plot.close()


def autoDetectDevices():
    """
    Auto detects PPS Analyzer devices.

    Searches through all ttyUSB devices in /dev and tries each one if it answers correctly to the
    connect message. The code is somewhat "hacky" (accessing PpsAnalyzer internals) but this is
    done for the reason of speed and to get nice logs.
    """
    logger.info("autoDetectDevices()")

    if sys.platform.startswith('win'):
        uarts = ['COM%s' % (i + 1) for i in range(256)]
    elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
        # this excludes your current terminal "/dev/tty"
        uarts = glob.glob('/dev/tty[A-Za-z]*')
    else:
        raise Exception("Unknown OS")
        
    devs = []
    for dev in uarts:
        logger.debug(f"trying {dev}")
        try:
            pa = PpsAnalyzer(port=dev)
            pa.uart = serial.Serial(port=dev, baudrate=1000000, timeout=0.5)
            pa.communicate("CC", [], numTries=1)
            logger.info(f"NTL device found at {dev}")
            # check if timestamper core is there (type 4)
            addr = 0x00000000
            while addr < 4096:
                rv = pa.readReg(addr)
                addr += 16
                if ((rv >> 16) & 0x0000FFFF) == 4:
                    logger.info(f"TSU found")
                    break
                elif (rv == 0) or (addr >= 4096):
                    logger.info(f"no TSU found, skipping")
                    raise Exception("end of table")
            devs.append(dev)
            logger.info(f"found device under {dev}")
        except Exception:
            pass  # ignore
        finally:
            if pa.uart:
                pa.uart.close()
                pa.uart = None
    if len(devs) == 0:
        raise Exception("No devices detected")
    return devs


if __name__ == '__main__':
    """
    Testing and example code.
    """
    # configure logging for this main method:
    logging.basicConfig(level=logging.INFO)

    logging.info("Creation and setup")
    devs = autoDetectDevices()
    pa = PpsAnalyzer(port=devs[0], isHeadless=True)
    
    pa.setUp([0, 1, 2, 3 , 4, 5, 6, 7, 8])

    for tsuIdx in range(9):
        logging.info("Accessing TSUs")
        pa.tsu[tsuIdx].setPolarity(isActiveHigh=True)
        logging.info(f"polarityActiveHigh={pa.tsu[tsuIdx].isPolarityActiveHigh()}")

        pa.tsu[tsuIdx].setCableDelay(0xFFFF)
        logging.info(f"cableDelay={pa.tsu[tsuIdx].getCableDelay()}")

        pa.tsu[tsuIdx].setEnable(True)
        logging.info(f"isEnabled={pa.tsu[tsuIdx].isEnabled()}")

    logging.info("Reading single time stamp")
    if pa.tsu[0].hasTimestamp():
        logging.info(f"timestamp={pa.tsu[0].readTimestamp()}")

    time.sleep(2)

    fileTime = datetime.now()
    filePrefix = str(fileTime.day) + "_" + str(fileTime.month) + "_" + str(fileTime.year) + "__" + str(fileTime.hour) + "_" + str(fileTime.minute) + "_" + str(fileTime.second) + "__"   
    logFile = filePrefix + "ts.log"
    pngFile = filePrefix + "ts.png"
    
    if len(sys.argv) > 1:
        duration = int(sys.argv[1])
    else:
        duration = 1000000000
    
    logging.info(f"Recording and plotting real data for {duration}s")
    ts = pa.recordTimeStamps(durationS=duration, fileName=logFile)
    pa.plotTimeStamps(data=ts, title=f"Recorded PPS Data ({ts['startTime']})",
                      ylimits=[None, None], fileName=pngFile)

    for tsuIdx in range(9):
        pa.tsu[tsuIdx].setEnable(False)


