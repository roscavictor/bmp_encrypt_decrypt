# bmp_encrypt_decrypt
## Short description
### Encrypting
- Transform the 2d image into a 1d array of pixels in __Lin__
- create an array of random numbers, initialise the first element with with the key in __key.txt__ 
  and initialise the numbers with xorshift32
-shuffle elements using the [Durstenfeld algorithm](https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle)
- create "vprim" array of pixels which contains the elements of the first pixel array v,
   but the elements are initialised  "according to the permutation"
- sv is the starting value from the key file, and we assign it in an int , "val" that's in an union that 
 contains unsigned char (r,g,b). This union makes "xor-ing" the pixels with random numbers easier, numbers which will be inserted in the "val" of the unions. In union y we initialise val with the h*w-th element from the array
 of random numbers
- create vsecond, initialise its first element, and afterwards, in union z, its value is initialised
   with the (h*w+i)-th element from the array of random numbers
- save array
### Decrypting
 - Mostly the same, just that when we encrypt, we first do the permutation and then xor them
  - Instead, we first xor them, and then do the permutation
