main
{
	str = "12Torben Weis";
	if ( str =~ /([A-Za-z]+) (.+)/ )
	{
		println("Match '%1' '%2' '%3'".arg( $0 ).arg( $1 ).arg( $2 ) );
	}
	else
	{
		println("No Match");
	}

	if ( str =~ s/[0-9]([0-9]+)/Hello (\1) / )
	{
		println("Match '%1'".arg( str ) );
	}
	else
	{
		println("No Match");
	}
	
}
