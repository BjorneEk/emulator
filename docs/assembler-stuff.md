

assembly syntax:

.global: lbl1, lbl2, lbl3

label = #10

.text:

label:
	ins arg1, arg2..

.data:
	string a = "assas"
	i8 b = -32
	u8 1

linker-script syntax:

sections (ro_memory, ra_memory, program)

endian: (little_endian, big_endian)

format: (bin, hex)

entry_point: section_name.label

entry_point_store: #0xFFFC //the adress on witch the entry point is stored

ro_memory { // real address
	org:	#0x9172;
	length:	#10;
}

ra_memory { // real address
	org:	#0x9172;
	length:	#10;
}

program (0xFFFFFFFF) // total output size
{ // result output file offset address
	pad 0x00			// fill output with 0 untill next piece of data
	org (section1_name): #0xFF	// place section1 data starting at address 0xFF
	put section2_name		// place section2 data after section1 (sizeof(section1) + 0xFF)
	pad 0x00 0xBABA			// pad with 0 from (sizeof(section1) + 0xFF) untill address 0xBABA
	put #0xFF			// place a byte here (0xBABA)
	put #0xFF #10			// place 10 bytes of #0xFF here (0xBABA + 1)
}