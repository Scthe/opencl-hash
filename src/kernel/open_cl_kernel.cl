__kernel void Kernel_1(
	__global const float* a,
	__global const float* b,
	__global float* c,
	int iNumElements)
{
	// get index into global data array
	int iGID = get_global_id(0);

	// bound check (equivalent to the limit on a 'for' loop for standard/serial C code
	if (iGID >= iNumElements){ 
		return; 
	}

	// add the vector elements
	c[iGID] = a[iGID] + b[iGID];
}

/*
letters = "acdegilmnoprstuw"
for i in range(26):
  # chr(ord('a')+1)
  c = chr(ord('a')+i)
  pos = letters.find(c)
  if pos ==-1: pos =0
  #print c+" "+str(pos)
  print str(pos)+","
  
*/

// data types:
// http://www.khronos.org/registry/cl/sdk/2.0/docs/man/xhtml/scalarDataTypes.html

__constant int lookup[26] = { 0, 0, 1, 2, 3, 0, 4, 0, 5, 0, 0, 6, 7, 8, 9, 10, 0, 11, 12, 13, 14, 0, 15, 0, 0, 0 };
__constant char letters[16] = {'a','c','d','e','g','i','l','m','n','o','p','r','s','t','u','w'};
__constant long target_hash = 910897038977002;

long hash(const char* c){
	long h = 7;
	for (unsigned int i = 0; i < 9; i++) {
		h = (h * 37 + lookup[c[i]-'a']);
	}
	return h;
}

__kernel void HashKernel( __global char* target, int base){
	// get index into global data array
	uint iGID = get_global_id(0); // 16^4
	
	// buffer
	char data[9];

	// deconstruct
	int from_base = 5;
	for (uint i = 0; i < from_base; i++){
		uint tmp = (base >> (i * 4)) & 15;
		data[i] = letters[tmp];
	}
	for (uint i = 0; i < 9-from_base; i++){
		uint tmp = (iGID >> (i * 4)) & 15;
		data[from_base+i] = letters[tmp];
	}
	
	long hash_ = hash(data);
	bool found = hash_ == target_hash;
	//if( found)break;
	if( hash_ == target_hash){
		found = true;
		target[0] = 'f';
		target[1] = 'o';
		target[2] = 'u';
		target[3] = 'n';
		target[4] = 'd';
		target[5] = ':';
		target[6] = ':';
		for (uint i = 0; i < 9; i++){ target[i+7] = data[i]; }
		target[16] = '\0';
		target[17] = '\0';
	}
}
