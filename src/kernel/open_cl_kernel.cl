//                              a  b  c  d  e  f  g  h  i  j  k  l  m  n  o  p   q  r   s   t   u   v  w   x  y  z
__constant int lookup[26] =   { 0, 0, 1, 2, 3, 0, 4, 0, 5, 0, 0, 6, 7, 8, 9, 10, 0, 11, 12, 13, 14, 0, 15, 0, 0, 0 };
__constant char letters[16] = {'a','c','d','e','g','i','l','m','n','o','p','r','s','t','u','w'};
__constant long target_hash = 910897038977002;
__constant long letter_count = 9;


long hash(const char* c){
	long h = 7;
	for (unsigned int i = 0; i < 9; i++) {
		h = (h * 37 + lookup[c[i]-'a']);
	}
	return h;
}

/**
 * @param target result buffer, expected length: letters
 * @param positions_to_fill each thread will only have to iterate over
 *          only last positions_to_fill letters
 */
__kernel
void main(__global char* target,
					__global int* found_flag,
					int cpu_iter_i,
					int letters_to_iter_over){
	uint global_id = get_global_id(0);

	// letters buffer
	char data[9]; // TODO char data[letter_count];

	/*
	// fill first (9-positions_to_fill) letters based on global_id
	for (uint i = letter_count-1; i > positions_to_fill; i--){ // ? positions_to_fill-1 ?
		uint letter_id = (global_id >> (i * 4)) & 0xf;
		data[i] = letters[letter_id]; // TODO remove letters array
	}
	*/

	data[0] = letters[(cpu_iter_i >> 0) & 0xf];
	data[1] = letters[(cpu_iter_i >> 4) & 0xf];
	data[2] = letters[(cpu_iter_i >> 8) & 0xf];
	data[3] = letters[(global_id >>  0) & 0xf];
	data[4] = letters[(global_id >>  4) & 0xf];
	data[5] = letters[(global_id >>  8) & 0xf];
	data[6] = letters[(global_id >> 12) & 0xf];
	// we will iterate over 7 & 8 letter

	// loop over all possible combinations for remaining (positions_to_fill) letters
	uint combinations = 1 << (4 * letters_to_iter_over); // positions*combinations/position
	for (uint i = 0; i < combinations; i++){
		/*
		// every position to fill -> fill based on i
		for (uint letter_idx = 0; letter_idx < letters_to_iter_over; letter_idx++){
			uint letter_id = (i >> (letter_idx * 4)) & 0xf;
			data[letter_idx] = letters[letter_id];
		}
		*/
		data[7] = letters[(i >> 0) & 0xf];
		data[8] = letters[(i >> 4) & 0xf];

		// calculate hash
		long hash_ = hash(data);
		if( hash_ == target_hash){
			// int old_value = atomic_cmpxchg(found_flag, 0, 1); // if none found before us
			// if(old_value == 0){
				for (uint i = 0; i < letter_count; i++){
					target[i] = data[i];
				}
			// }
			break;
		}
	}

}
