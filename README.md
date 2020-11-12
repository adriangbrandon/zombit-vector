# cds-template

Template for creating a Compact Data Structure project using SDSL library.


####A little example

This code shows how we can create a simple data structure (table_bitmap) that receives as an input an integer pair vector and recognizes each pair as (index, data). In order to store this information, this code uses a bit_vector and an int_vector respectively.

The main function loads 5 random pairs into this data structure  and outputs what positions were filled and its content. As it can be seen in the constructor, the length of the bit_vector will be the maximum index included in the input and the length of the int_vector will be the number of pairs received. 

Table_bitmap.hpp includes also some basic concepts that you will use in all your CDS projects as type definitions, SDSL library usage or class functions management.

####Installing the example

- git clone 
- ./cds-template/compile.sh (if needed, use chmod +x path/compile.sh)
- ./cds-template/build/template_executable