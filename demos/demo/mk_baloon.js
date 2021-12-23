function e(s) {
   let b = 0;
   for(let t=0;t<s.length;t++) {
      
      if(s.charAt(t) != ".") {         
         b |= 1<<(7-t);
      }      
   }
   let msg = `00000000`+b.toString(2);   
   console.log(`   0b${msg.substr(msg.length-8)},`);
}

console.log("// file generated automatically by mkfont.js -- do not edit");

/*
e("....XXXX"); e("XXX.....");
e("..XXXXXX"); e("XXXXX...");
e(".XXXXXXX"); e("XXXXXX..");
e(".XXXXX.."); e("XXXXXX..");
e("XXXXX.XX"); e("..XXXXX.");
e("XXXXX.XX"); e("XXXXXXX.");
e("XXXXX.XX"); e("..XXXXX.");
e(".XXXXX.."); e(".XXXXX..");
e(".XXXXXXX"); e("XXXXXX.."); //
e(".XXXXXXX"); e("XXXXXX..");
e(".X.XXXXX"); e("XXXX.X..");
e("..X.XXXX"); e("XXX.X...");
e("..X..XXX"); e("XX..X...");
e("...X..XX"); e("X..X....");
e("...X..XX"); e("X..X....");
e("....X..X"); e("..X.....");
e("....X..X"); e("..X.....");  //
e(".....XXX"); e("XX......");
e(".....XXX"); e("XX......");
e(".....XXX"); e("XX......");
e("......XX"); e("X.......");
e("........"); e("........");
e("........"); e("........");
e("........"); e("........");
e("........"); e("........"); //
e("........"); e("........");
e("........"); e("........");
e("........"); e("........");
e("........"); e("........");
e("........"); e("........");
e("........"); e("........");
e("........"); e("........");
*/

e("....XXXX"); 
e("..XXXXXX"); 
e(".XXXXXXX"); 
e(".XXXXX.."); 
e("XXXXX.XX"); 
e("XXXXX.XX"); 
e("XXXXX.XX"); 
e(".XXXXX.."); 

e(".XXXXXXX");  //
e(".XXXXXXX"); 
e(".X.XXXXX"); 
e("..X.XXXX"); 
e("..X..XXX"); 
e("...X..XX"); 
e("...X..XX"); 
e("....X..X"); 

e("XXX.....");
e("XXXXX...");
e("XXXXXX..");
e("XXXXXX..");
e("..XXXXX.");
e("XXXXXXX.");
e("..XXXXX.");
e("XXXXXX..");

e("XXXXXX..");
e("XXXXXX..");
e("XXXX.X..");
e("XXX.X...");
e("XX..X...");
e("X..X....");
e("X..X....");
e("..X.....");

e("....X..X");   //
e(".....XXX"); 
e(".....XXX"); 
e(".....XXX"); 
e("......XX"); 
e("........"); 
e("........"); 
e("........"); 

e("........");  //
e("........"); 
e("........"); 
e("........"); 
e("........"); 
e("........"); 
e("........"); 
e("........"); 

e("..X.....");
e("XX......");
e("XX......");
e("XX......");
e("X.......");
e("........");
e("........");
e("........");

e("........");
e("........");
e("........");
e("........");
e("........");
e("........");
e("........");
e("........");