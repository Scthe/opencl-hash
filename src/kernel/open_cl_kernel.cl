//
// LETTER_COUNT - will be provided during program build
//

__constant int lookup[26] = {
	0, 0, 1, 2, 3, 0, 4, 0, 5, 0, 0, 6, 7, 8, 9, 10, 0, 11, 12, 13, 14, 0, 15, 0, 0, 0 };
//a  b  c  d  e  f  g  h  i  j  k  l  m  n  o  p   q  r   s   t   u   v  w   x  y  z

// TODO remove letters array
__constant char letters[16] = {'a','c','d','e','g','i','l','m','n','o','p','r','s','t','u','w'};

long hash(const char* c){
	long h = 7;
	for (unsigned int i = 0; i < LETTER_COUNT; i++) {
		h = (h * 37 + lookup[c[i]-'a']);
	}
	return h;
}

__kernel
void main(__global char* target,
					// __global int* found_flag,
					long target_hash,
					int letters_from_global_id,
					int cpu_iter_i,
					int letters_to_iter_over){
	uint global_id = get_global_id(0);
	uint letters_from_cpu_iter_i = LETTER_COUNT - letters_to_iter_over - letters_from_global_id;
	char data[LETTER_COUNT]; // letters buffer

	// fill letters based on cpu_iter_i
	for (uint i = 0; i < letters_from_cpu_iter_i; i++){
		data[i] = letters[(cpu_iter_i >> (i*4)) & 0xf];
	}

	// fill letters based on global_id
	for (uint i = 0; i < letters_from_global_id; i++){
		uint ii = letters_from_cpu_iter_i + i;
		data[ii] = letters[(global_id >> (i * 4)) & 0xf];
	}

	uint base_idx = letters_from_cpu_iter_i + letters_from_global_id;
	uint combinations = 1 << (4 * letters_to_iter_over);
	for (uint i = 0; i < combinations; i++){
		// fill out last letters
		for (uint idx = 0; idx < letters_to_iter_over; idx++){
			uint ii = base_idx + idx;
			data[ii] = letters[(i >> (idx * 4)) & 0xf];
		}

		// calculate hash
		long hash_value = hash(data);
		if( hash_value == target_hash){
			// int old_value = atomic_cmpxchg(found_flag, 0, 1); // if none found before us
			// if(old_value == 0){
				for (uint i = 0; i < LETTER_COUNT; i++){
					target[i] = data[i];
				}
			// }
			break;
		}
	}

}
