pot( in x, inout o )
{
	print( o );
	o = x * 2;
}

pot2( in x, inout o )
{
	print( o );
	o = x * 2;
}

mul( in x, in y = 10 )
{
	x = 4;
	a = 7;
	print( x, y, x*y );
	return 5*a;
}

main
{
	print("Hallo Welt!" );
	mul( 5, 6 );
	a = 5;
	x = mul( a, 3*3 );
	b = 8;
	print( a, b, a*b );

	a = 1;
	while( a != 16 )
	{
		print( a );
		a = a * 2;
	}
	print( "Ende", a );
	mul( 8 );
	o = 0;
	pot( 23, o );
	print( "out:", o );
	pot2( 27, o );
	print( "inout:", o );
}