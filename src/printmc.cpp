/*
#include "mex.hpp"
#include "mexAdapter.hpp"
using namespace matlab::data;
using matlab::mex::ArgumentList;

class MexFunction : public matlab::mex::Function {
    public:
        void operator()(ArgumentList outputs, ArgumentList inputs) {
            matlab::data::ArrayFactory factory;
            size_t n = inputs[0][0], m = inputs[1][0];
            double *data = new double[n * m];
            //buffer_ptr_t<double> buffer = factory.createBuffer<double>(n * m);
            //double* data = buffer.get();
            for (int i = 0; i < (n * m); i++) {
                data[i] = static_cast<double>(i);
            }
            buffer_deleter_t deleter = [](void* ptr) { delete(ptr); };
            buffer_ptr_t<double> buffer = buffer_ptr_t<double>(data, deleter);
            //TypedArray<double> array = factory.createArrayFromBuffer<double>({3, 3}, std::move(buffer));
            outputs[0] = factory.createArrayFromBuffer<double>({n, m}, std::move(buffer), MemoryLayout::ROW_MAJOR);
        }
};
*/

#include "mex.hpp"
#include "mexAdapter.hpp"
#include <iomanip>
//#include <stdint>
//#include <iostream>
using namespace matlab::data;
using matlab::mex::ArgumentList;

struct ih {
    void *ad1;
    //std::int64_t fl;
    void *fl;
    void *ad2;
    //std::int64_t fl2;
    void *fl2;
    void *dims;
    void *ad4;
    void *ad5;
    void *d8;
    void *d9;
    void *d10;
};

void _printmem(void* addr, int nbytes) {
    unsigned char *mem = static_cast<unsigned char *>(addr);
    std::cout << "looking at memory at " << addr << "\n";
    for (int i = 0; i < nbytes; i++) {
        std::cout << std::hex << std::setfill('0') << std::setw(2) << int(mem[i]) << " ";
        if ((i+1) % 16 == 0) std::cout << "\n";
    } std::cout << "\n";
}

class MexFunction : public matlab::mex::Function {
    public:
        void operator()(ArgumentList outputs, ArgumentList inputs) {
            matlab::data::ArrayFactory factory;
            void* arr = static_cast<void*>(&inputs[0]);
            std::cout << "Array obj at " << arr << "\n";
            //std::shared_ptr<matlab::data::impl::ArrayImpl> pimp = 
            //        matlab::data::detail::Access::getImplPtr< std::shared_ptr<matlab::data::impl::ArrayImpl> >(inputs[0]);
            //matlab::data::impl::ArrayImpl* pimp = matlab::data::detail::Access::getImplForWorkspace(inputs[0]);
            matlab::data::impl::ArrayImpl* imp = matlab::data::detail::Access::getImpl<matlab::data::impl::ArrayImpl>(inputs[0]);
            //std::cout << "pimp = " << pimp << ", imp = " << imp << "\n";
            typedef void (*ArrayDestroyFcnPtr)(impl::ArrayImpl*);
            static const ArrayDestroyFcnPtr deleter =
                matlab::data::detail::resolveFunction<ArrayDestroyFcnPtr>(matlab::data::detail::FunctionType::ARRAY_DESTROY);
            std::cout << "imp = " << imp << ", deleter = " << deleter << "\n";
            void* addr = static_cast<void*>(imp);
            int nbytes = 96;
            if (inputs.size() > 1) {
                nbytes = inputs[1][0];
            }
            _printmem(addr, nbytes);
            struct ih* hd = static_cast<struct ih*>(addr);
            _printmem(hd->ad2, 256);
//            _printmem(hd->ad2, nbytes);
            //_printmem(hd->dims, 32);
            //_printmem(hd->ad4, nbytes);
            //_printmem(hd->ad5, nbytes);
            struct ih* hd2 = static_cast<struct ih*>(hd->ad2);
            _printmem(hd2->ad2, nbytes); // mxArray copy
            //_printmem(hd2->d9, 32);
            //struct ih* hd3 = static_cast<struct ih*>(hd2->d9);
            //_printmem(hd3->fl, nbytes);
//            struct ih* hd3 = static_cast<struct ih*>(hd2->d10);
//            _printmem(hd3->ad1, nbytes);
      //      _printmem(hd2->d9, nbytes);
//            _printmem(hd2->d10, nbytes);
//            struct ih* hd3 = static_cast<struct ih*>(hd2->d9);
   //         _printmem(hd3->d9, nbytes);
//            struct ih* hd4 = static_cast<struct ih*>(hd3->fl);
//            _printmem(hd4->d9, nbytes);
//            _printmem(hd4->d10, nbytes);
/*

    mxUint64 *addr = (mxUint64*) &prhs[0];
    mexPrintf("looking at memory at %llx\n", *addr);
    unsigned char *mem = (unsigned char *) addr[0];
    for (int i = 0; i < 96; i++) {
        mexPrintf("%.2x ", mem[i]);
        if ((i+1) % 16 == 0) printf("\n");
    }
    mexPrintf("\n");

  
            double *data = new double[9];
            for (int i = 0; i < 9; i++) {
                data[i] = static_cast<double>(i);
            }
            buffer_deleter_t deleter = [](void* ptr) { delete(ptr); };
            buffer_ptr_t<double> buffer = buffer_ptr_t<double>(data, deleter);
            std::cout << "Original buffer address = " << buffer.get() << "\n";
            std::cout << "Custom deleter address = " << deleter << "\n";
            TypedArray<double> array = factory.createArrayFromBuffer<double>({3, 3}, std::move(buffer));
            buffer_ptr_t<double> array_buffer = array.release();
            buffer_deleter_t array_deleter = array_buffer.get_deleter();
            std::cout << "Released array buffer address = " << array_buffer.get() << "\n";
            std::cout << "Created array deleter address = " << array_deleter << "\n";
            // We have to assign the memory to an output variable otherwise Matlab will try to delete
            // the buffer memory with its own deleter and this causes a heap memory error.
            outputs[0] = factory.createArrayFromBuffer<double>({3, 3}, std::move(array_buffer)); */
    }
};


/* printmem.cpp */
/*
#include "mex.h"
#include "matrix.h"
#include <cstdint> 

struct mxArray_header_2020a { // 96 bytes long
    void *unknown_address;
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

void mexFunction( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
    if (nrhs < 1) mexErrMsgTxt("One input required.");
    mxUint64 *addr = (mxUint64*) &prhs[0];
    mexPrintf("looking at memory at %llx\n", *addr);
    unsigned char *mem = (unsigned char *) addr[0];
    for (int i = 0; i < 96; i++) {
        mexPrintf("%.2x ", mem[i]);
        if ((i+1) % 16 == 0) printf("\n");
    }
    mexPrintf("\n");
    void* arr = static_cast<void*>(mem);
    struct mxArray_header_2020a* mx = static_cast<struct mxArray_header_2020a*>(arr);
    mexPrintf("mxArray at %llx, pr = %llx, %llx\n", mx,  mx->pr);
    mexPrintf("ClassID is %i\n", mx->ClassID);
    mexPrintf("size = [");
    mwSize ndims = mxGetNumberOfDimensions(prhs[0]);
    if (ndims > 2) {
        std::int64_t* sizes = mx->Mdims.dims;
        for (size_t ii = 0; ii < ndims; ii++) { mexPrintf("%i, ", sizes[ii]); }
	} else {
        mexPrintf("%i, %i", mx->Mdims.M, mx->Nndim.N);
	}
    mexPrintf("]\n");
}
*/
/*
void mexFunction( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
  if (nrhs < 1 || !mxIsUint64(prhs[0]) || mxIsEmpty(prhs[0]))
    mexErrMsgTxt("First argument must be a uint64 memory address");
  mxUint64 *addr = mxGetUint64s(prhs[0]);
  mexPrintf("looking at memory at %llx\n", *addr);
  unsigned char *mem = (unsigned char *) addr[0];
  if (nrhs < 2 || !mxIsDouble(prhs[1]) || mxIsEmpty(prhs[1]))
    mexErrMsgTxt("Second argument must be a double-type integer byte size.");
  unsigned int nbytes = static_cast<unsigned int>(mxGetScalar(prhs[1]));
  for (int i = 0; i < nbytes; i++) {
    mexPrintf("%.2x ", mem[i]);
    if ((i+1) % 16 == 0) printf("\n");
  }
  mexPrintf("\n");
}
*/
/*
#include "mex.h"
#include "matrix.h"
void mexFunction( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
   if (nrhs < 1) mexErrMsgTxt("One input required.");
   plhs[0] = mxCreateNumericMatrix(1, 1, mxUINT64_CLASS, mxREAL);
   mxUint64 *out = mxGetUint64s(plhs[0]);
   out[0] = (mxUint64) prhs[0];
}
*/
