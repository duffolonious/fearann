#! /usr/bin/python
 
# (C) 2006 Manuel A. Fernandez Montecelo <mafm@users.sourceforge.net>
#
# This script is free software and is not covered by any particular
# license, you have unlimited permission to copy, distribute and
# modify it.
 
######################################################################
# <Basic configuration>
#
# These are flags that you may want to edit, but only if you know what
# you're doing.  We developers put them separated here mainly to be
# able to change them easily instead of searching through the code,
# not to encourage users to change it.  Better use command like
# options (in example to specify the processor, instead of modifying
# the value of MARCH) or edit Jamrules file instead when possible, so
# you maintain here the factory settings.  Changing this values may
# easily lead to problems difficult to debug, so the first that
# developers will ask you is to use factory settings...
#
######################################################################

MARCH="notset"
LDFLAGS="-lstdc++ -lpthread -lm"
BASE_CXXFLAGS="-pipe -ansi -std=c++98 -Wall -Wextra -Wno-unused-parameter -Wpointer-arith -Wreturn-type -Wcast-qual -Wswitch -Wshadow -Wcast-align -Wwrite-strings -Wchar-subscripts -Wredundant-decls -Woverloaded-virtual -Wdeprecated "
CXXFLAGS_DEBUG="-g3 -O0"
CXXFLAGS_OPTCOMPAT="-g0 -O2 -ffast-math"
CXXFLAGS_OPTFULL="-g0 -O3 -ffast-math"

######################################################################
# </Basic configuration>
######################################################################
 

######################################################################
# <Special configuration>
#
# These are flags that you should not edit unless you know that these
# lines are causing problems and that you have the correct fix.
# Better edit Jamrules file instead, if possible.
#
######################################################################

# file of additional flags for jam
JAMRULES_FILE = "Jamrules"

# commands (name, actual command) to be executed when checking for
# needed tools and library versions
cmdGCC = ['G++', 'g++ -v']
cmdJam = ['Jam', 'jam -v']
cmdPKGCFG = ['pkg-config', 'pkg-config --version']
cmdOSG = ['OSG', 'pkg-config openscenegraph --atleast-version=1.0.0']
cmdCflagsOSG = ['OSG.CXXFLAGS', 'pkg-config openscenegraph --cflags']
cmdLflagsOSG = ['OSG.LDFLAGS', 'pkg-config openscenegraph --libs']
cmdOSGCAL = ['OSGCAL', 'pkg-config osgcal --atleast-version=0.1.41']
cmdCflagsOSGCAL = ['OSGCAL.CXXFLAGS', 'pkg-config osgcal --cflags']
cmdLflagsOSGCAL = ['OSGCAL.LDFLAGS', 'pkg-config osgcal --libs']
cmdCAL3D = ['CAL3D', 'pkg-config cal3d --atleast-version=0.11']
cmdCflagsCAL3D = ['CAL3D.CXXFLAGS', 'pkg-config cal3d --cflags']
cmdLflagsCAL3D = ['CAL3D.LDFLAGS', 'pkg-config cal3d --libs']
cmdCEGUI = ['CEGUI', 'pkg-config CEGUI-0 --atleast-version=0.8.0']
cmdCflagsCEGUI = ['CEGUI.CXXFLAGS', 'pkg-config CEGUI-0 --cflags']
cmdLflagsCEGUI = ['CEGUI.LDFLAGS', 'pkg-config CEGUI-0 --libs']
cmdCEGUIOPENGL = ['CEGUI-OpenGL', 'pkg-config CEGUI-0-OPENGL --atleast-version=0.5.0']
cmdCflagsCEGUIOPENGL = ['CEGUIOPENGL.CXXFLAGS', 'pkg-config CEGUI-0-OPENGL --cflags']
cmdLflagsCEGUIOPENGL = ['CEGUIOPENGL.LDFLAGS', 'pkg-config CEGUI-0-OPENGL --libs']
######################################################################
# </Special configuration>
######################################################################

######################################################################
# <Helper functions and declarations>
######################################################################
 
import os,sys,string,commands

def writeToFile(file, line):
    """Writes a line to the file"""
    f = open(file, "a")
    f.write(line + " ;\n")
    f.close()

def checkToolLib(cmd):
    """Check for tools/libs needed to compile"""
    (status, output) = commands.getstatusoutput(cmd[1])
    line = " - " + cmd[0] + ": "
    if status == 0:
	print line + "OK"
    else:
	print line + "failed\n\t(" + str(cmd[1]) + ")"
	print "Aborting"
	os._exit(1)

def addLibFlags(cmd):
    """Add lib flags needed to compile or link"""
    (status, output) = commands.getstatusoutput(cmd[1])
    line = cmd[0] + ' = "' + output + '"'
    writeToFile(JAMRULES_FILE, line)

######################################################################
# </Helper functions and declarations>
######################################################################

######################################################################
# <Execution>
######################################################################

#
# parse options
#
from optparse import OptionParser
parser = OptionParser()
parser.add_option("-m", "--mode",
		  action="store", type="string", dest="MODE", default="O",
                  help="compilation mode: O|F|D (Optimization|Full opt.|Debug) [default: O]")
parser.add_option("-p", "--processor",
		  action="store", type="string", dest="PROCESSOR", default=MARCH,
                  help="processor.  When specified and using full optimization mode,"
			" it's the value for '-march' option in GCC.  See the man page"
			" for the list of available processors.")
parser.add_option("-c", "--with-client",
		  action="store_true", dest="CLIENT", default=True,
                  help="compile client [default]")
parser.add_option("-C", "--without-client",
		  action="store_false", dest="CLIENT", default=True,
                  help="don't compile client")
parser.add_option("-s", "--with-server",
		  action="store_true", dest="SERVER", default=False,
                  help="compile server")
parser.add_option("-S", "--without-server",
		  action="store_false", dest="SERVER", default=False,
                  help="don't compile server [default]")
parser.add_option("-b", "--with-bot",
		  action="store_true", dest="BOT", default=False,
                  help="compile bot")
parser.add_option("-B", "--without-bot",
		  action="store_false", dest="BOT", default=False,
                  help="don't compile bot [default]")
(options, args) = parser.parse_args()

#
# remove Jamrules file if present
#
print "*** Starting the configuration script (output: " + JAMRULES_FILE + ")"
if os.path.exists(JAMRULES_FILE):
    os.remove(JAMRULES_FILE)

#
# determine compile mode (debug, optimizations, ...)
#
print "\n*** Setting building parameters"
if options.PROCESSOR:
    MARCH=options.PROCESSOR

if options.MODE == "D":
    COMPILE_MODE="Debug"
    CXXFLAGS=BASE_CXXFLAGS + CXXFLAGS_DEBUG
elif options.MODE == "O":
    COMPILE_MODE="Optimization"
    CXXFLAGS=BASE_CXXFLAGS + CXXFLAGS_OPTCOMPAT
elif options.MODE == "F":
    if MARCH == "notset":
        print "PROCESSOR must be set when using Full Optimization mode (this script can't guess it), aborting"
        os._exit(1)
    else:
        COMPILE_MODE="Full optimization"
        CXXFLAGS=BASE_CXXFLAGS + CXXFLAGS_OPTFULL + " -march=" + MARCH
else:
    print "Compilation mode not set, aborting"
    os._exit(1)

print " - Building mode: " + COMPILE_MODE
if options.MODE == "F":
    print "   - Target processor: " + MARCH
print " - Compile flags: " + CXXFLAGS
writeToFile(JAMRULES_FILE, 'CXXFLAGS = "' + CXXFLAGS + '"')
writeToFile(JAMRULES_FILE, 'LDFLAGS = "' + LDFLAGS + '"')

#
# modules to build
#
print "\n*** Setting modules to build"
if options.CLIENT:
    print " - Client"
    writeToFile(JAMRULES_FILE, 'BUILD_CLIENT = yes')
else:
    writeToFile(JAMRULES_FILE, 'BUILD_CLIENT = no')
if options.SERVER:
    print " - Server"
    writeToFile(JAMRULES_FILE, 'BUILD_SERVER = yes')
else:
    writeToFile(JAMRULES_FILE, 'BUILD_SERVER = no')
if options.BOT:
    print " - Bot"
    writeToFile(JAMRULES_FILE, 'BUILD_BOT = yes')
else:
    writeToFile(JAMRULES_FILE, 'BUILD_BOT = no')

#
# find commands and library versions needed
#
print "\n*** Checking for tools needed"
checkToolLib(cmdGCC)
checkToolLib(cmdJam)
checkToolLib(cmdPKGCFG)
print "\n*** Checking for libraries (and suitable versions) needed"
print " Common: "
writeToFile(JAMRULES_FILE, 'XERCES.AVAILABLE = "yes"')
writeToFile(JAMRULES_FILE, 'XERCES.CXXFLAGS = ""')
writeToFile(JAMRULES_FILE, 'XERCES.LDFLAGS = "-lxerces-c"')
print " - Xerces-C: OK"
if options.CLIENT:
    print " Client:"
    checkToolLib(cmdOSG)
    addLibFlags(cmdCflagsOSG)
    addLibFlags(cmdLflagsOSG)
    checkToolLib(cmdCEGUI)
    addLibFlags(cmdCflagsCEGUI)
    addLibFlags(cmdLflagsCEGUI)
    checkToolLib(cmdCEGUIOPENGL)
    addLibFlags(cmdCflagsCEGUIOPENGL)
    addLibFlags(cmdLflagsCEGUIOPENGL)
    checkToolLib(cmdOSGCAL)
    addLibFlags(cmdCflagsOSGCAL)
    addLibFlags(cmdLflagsOSGCAL)
    checkToolLib(cmdCAL3D)
    addLibFlags(cmdCflagsCAL3D)
    addLibFlags(cmdLflagsCAL3D)
if options.SERVER:
    print " Server: "
    checkToolLib(cmdOSG)
    addLibFlags(cmdCflagsOSG)
    addLibFlags(cmdLflagsOSG)
    writeToFile(JAMRULES_FILE, 'POSTGRESQL.AVAILABLE = "yes"')
    writeToFile(JAMRULES_FILE, 'POSTGRESQL.CXXFLAGS = "-DHAVE_POSTGRESQL"')
    writeToFile(JAMRULES_FILE, 'POSTGRESQL.LDFLAGS = "-lpq"')
    print " - PostgreSQL: OK"

#
# finish
#
writeToFile(JAMRULES_FILE, 'JAMRULES_COMPLETE = yes')
print "\n*** Everything seems all right, now type \'jam\' to compile"

######################################################################
# </Execution>
######################################################################
