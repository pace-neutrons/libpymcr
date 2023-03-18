/* printmem.cpp */
#include "mex.h"
#include "matrix.h"
#include <cstdint>

struct mxArray_header_2020a { // 96 bytes long
    std::int32_t *refcount;  // Pointer to the number of shared copies
    void *unknown2;
    std::int64_t ClassID;    // https://mathworks.com/help/matlab/apiref/mxclassid.html
    std::int64_t flags;      // ???
    union {
        std::int64_t M;      // Row size for 2D matrices, or
        std::int64_t *dims;  // Pointer to dims array for nD > 2 arrays
    } Mdims;
    union {
        std::int64_t N;      // Column size for 2D matrices, or
        std::int64_t *ndims; // Number of dimemsions for nD > 2 arrays
    } Nndim;
    void *unknown_addr2;     // Something related to structs and cells
    void *pr;                // Pointer to the data
    void *unknown_addr3;     // Something related to structs or sparse
    void *unknown_addr4;     // Something related to sparse
    void *unknown3;          // Seems to be zero
    void *unknown4;          // Seems to be zero
};

void _printmem(void* addr, int nbytes) {
    mexPrintf("looking at memory at %llx\n", addr);
    unsigned char *mem = (unsigned char *) addr;
    for (int i = 0; i < nbytes; i++) {
        mexPrintf("%.2x ", mem[i]);
        if ((i+1) % 16 == 0) printf("\n");
    }
    mexPrintf("\n");
}

void mexFunction( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
    if (nrhs < 1) mexErrMsgTxt("One input required.");
    mxUint64 *addr = (mxUint64*) &prhs[0];
    unsigned int nbytes = 96;
    if (nrhs > 1)
        nbytes = static_cast<unsigned int>(mxGetScalar(prhs[1]));
    void* arr = (void*) addr[0];
    
    struct mxArray_header_2020a* mx = static_cast<struct mxArray_header_2020a*>(arr);
    mexPrintf("mxArray at %llx, pr = %llx\n", mx,  mx->pr);
    mexPrintf("ClassID is %i\n", mx->ClassID);
    if (mx->refcount == NULL)
        mexPrintf("Refcount is NULL\n");
    else
        mexPrintf("Refcount is %i\n", *mx->refcount);
    mexPrintf("size = [");
    mwSize ndims = mxGetNumberOfDimensions(prhs[0]);
    if (ndims > 2) {
        std::int64_t* sizes = mx->Mdims.dims;
        for (size_t ii = 0; ii < ndims; ii++) { mexPrintf("%i, ", sizes[ii]); }
	} else {
        mexPrintf("%i, %i", mx->Mdims.M, mx->Nndim.N);
	}
    mexPrintf("]\n");

    _printmem(arr, nbytes);
    //if (mx->unknown_address)
    //    _printmem(mx->unknown_address, nbytes);
    //_printmem(mx->pr, nbytes);
    mexPrintf("First element = %f\n", *(double*)mx->pr);
}
