#!/usr/bin/env kross

"""
Python script that provides additional formula-functions
to KCells.

The script could be used in two ways;

    1. Embedded in KCells by execution via the "Tools=>Scripts"
       menu or from the "Tools=>Script Manager". The functions.py
       script may not be visible if not installed. So, for testing
       better use the second way;

    2. Run KCells with;

            # make the script executable
            chmod 755 `kde4-config --install data`/kcells/scripts/functions/functions.py
            # run KCells
            kcells --scriptfile `kde4-config --install data`/kcells/scripts/functions/functions.py

(C)2006 Sebastian Sauer <mail@dipe.org>
http://kross.dipe.org
http://www.koffice.org/kcells
Dual-licensed under LGPL v2+higher and the BSD license.
"""

class Functions:
    """ The Functions class adds some KCells formula functions on
    the fly and provides them to KCells. """

    def __init__(self, scriptaction):
        """ Some initial work like the import of the Kross and KCells functionality
        and test functions are added to demonstrate the usage. """

        import os, sys

        try:
            import Kross
        except:
            raise "Failed to import the Kross module."

        self.scriptaction = scriptaction
        #self.currentpath = self.scriptaction.currentPath()

        self.embeddedInKCells = False
        try:
            import KCells
            self.kcells = KCells
            self.embeddedInKCells = True
        except ImportError:
            try:
                import Kross
                self.kcells = Kross.module("kcells")
            except ImportError:
                raise "Failed to import the KCells Kross module. Please run this script with \"kross thisscriptfile.py\""

        self.addTestFunctions()

    def addTestFunctions(self):
        """ This method adds a new scripted formula function to KCells. """

        # create the new formula function "SCRIPT_TEST1"
        functest1 = self.kcells.function("SCRIPT_TEST1")
        # set the minimal number of parameters
        functest1.minparam = 1
        # set the maximal number of parameters, -1 means unlimited.
        functest1.maxparam = 1
        # set the comment displayed at "Help"
        functest1.comment = (
            "The SCRIPT_TEST1() function demonstrates how to use scripting"
            "functions. All it does is to take a string as argument and"
            "return the same string."
        )
        # set the syntax which is displayed at "Help".
        functest1.syntax = "SCRIPT_TEST1(string)"
        # set details about the parameter the formula functions expects.
        functest1.addParameter("String", "The string that should be returned")
        # add an example displayed at "Help".
        functest1.addExample("SCRIPT_TEST1(\"Some string\")")

        # this python function will be called by the KCells formula function
        def functest1callback(argument):
            # just return the first argument
            functest1.result = "%s" % argument[0]
        # connect the python function with the KCells formula function
        functest1.connect("called(QVariantList)", functest1callback)

        # and finally register the function to be able to use it within KCells
        functest1.registerFunction()

Functions( self )
