#!/usr/bin/env kross

// Print something on the console.
println("Let's start the KjsEmbed Unittest :-)");

function UnitTest()
{
	var numpassed = 0;
	var numfailed = 0;

	this.passed = function(actual, expected) {
		numpassed = numpassed + 1;
	}

	this.failed = function(actual, expected) {
		println("FAILED actual=" + actual + " expected=" + expected);
		numfailed = numfailed + 1;
	}

	this.assert = function(actual, expected) {
		if(actual == expected)
			this.passed(actual, expected);
		else
			this.failed(actual, expected);
	}

	this.printResult = function() {
		println("Tests passed: " + numpassed);
		println("Tests failed: " + numfailed);
	}
}

tester = new UnitTest();

// We have 2 instances of TestObject which is inherits QObject.
var testobj1 = TestObject1
var testobj2 = TestObject2

// object
tester.assert(testobj1, "TestObject");
tester.assert(testobj2, "TestObject");
tester.assert(testobj1.name(), "TestObject1");
tester.assert(testobj2.name(), "TestObject2");

// bool
tester.assert(testobj1.func_bool_bool(true), true);
tester.assert(testobj1.func_bool_bool(false), false);

// int
tester.assert(testobj1.func_int_int(0), 0);
tester.assert(testobj1.func_int_int(177321), 177321);
tester.assert(testobj1.func_int_int(-98765), -98765);

// uint
tester.assert(testobj1.func_uint_uint(0), 0);
tester.assert(testobj1.func_uint_uint(177321), 177321);

// double
tester.assert(testobj1.func_double_double(0.0), 0.0);
tester.assert(testobj1.func_double_double(1773.2177), 1773.2177);
tester.assert(testobj1.func_double_double(-548993.271993), -548993.271993);

// longlong
//TODO segfault
//tester.assert(testobj1.func_qlonglong_qlonglong(0), 0);
//tester.assert(testobj1.func_qlonglong_qlonglong(7379), 7379);
//tester.assert(testobj1.func_qlonglong_qlonglong(-6384673), -6384673);
//tester.assert(testobj1.func_qlonglong_qlonglong(678324787843223472165), 678324787843223472165);

// ulonglong
//TODO segfault
//tester.assert(testobj1.func_qulonglong_qulonglong(0), 0);
//tester.assert(testobj1.func_qulonglong_qulonglong(378972), 378972);

// bytearray
//TODO func_qbytearray_qbytearray-method returns always an empty bytearray
//tester.assert(testobj1.func_qbytearray_qbytearray("  Some String as ByteArray  "), "  Some String as ByteArray  ");
//tester.assert(testobj1.func_qbytearray_qbytearray(" \0\n\r\t\s\0 test "), " \0\n\r\t\s\0 test ");

// string
tester.assert(testobj1.func_qstring_qstring(""), "");
tester.assert(testobj1.func_qstring_qstring(" "), " ");
tester.assert(testobj1.func_qstring_qstring(" Another \n\r Test!   $%&\" "), " Another \n\r Test!   $%&\" ");

// stringlist
//TODO Cast failure QStringList value Type 6
//var a = new Array("string1","string");
//tester.assert(testobj1.func_qstringlist_qstringlist(a), a);

// variantlist
//TODO Cast failure QVariantList value Type 6
//var a = new Array("string1","string");
//tester.assert(testobj1.func_qvariantlist_qvariantlist(a), a);

// print the test-results
tester.printResult();

// Create a dialog and show it.
//var frame = new Widget("QFrame", this );
//frame.frameShape = frame.StyledPanel;
//frame.frameShadow = frame.Sunken;
//frame.lineWidth = 4;
//frame.show();
//exec();
