main
{
	println("-------1");
	app = findApplication( "kspread*" );
        doc = app.getDocuments() [0];
	println("-------2");
	map = doc.map();
	//	count = app.documentCount();
	// 	println( count );
	// 	doc = app.document( 0 );
	println("-------3");
	tcount = map.tableCount();
	println( tcount );
	table = map.table( 0 );
	println("--------4");
	name = table.name();
	println( name );
	println( map.tableNames() );
	r = QRect();
	r.left = 2;
	r.left = 2;
	r.width = 3;
	r.height = 4;
	table.setSelection( r );
	cell = table.cell( 1, 1 );
	cell.setText( "Hallo" );
        for( y = 2; y < 10; ++y )
	{
		cell = table.cell( 1, y );
		cell.setValue( y );
	}
	
        map.table( "Table1" ).cell("C1").setBgColor(192,255,192);
	cell=table.cell("C1");
	cell.setText("KSpread");
	
        for ( y = 2; y < 10; ++y)
        {
        	cell = table.cell( 3, y );
        	cell.setBgColor(table.cell("C1").bgColor());
        	
        	if((y-5)<=0)
		{
		//color = blue
		cell.setTextColor(0,0,255);
		}
		else
		{
		//color = red
		cell.setTextColor(255,0,0);
		}
		cell.setValue(y-5);
        }
	
	map.table( "Table1" ).cell("B1").setText( "Linux" );
	map.Table1().cell("B2").setText( "Torben" );
	map.Table1().B3().setText( "KDE" );
	map.Table1.B4.setText( "KOffice");
	t = map.insertTable( "NewTable" );
	t.A1().setText( "Wow" );
}
