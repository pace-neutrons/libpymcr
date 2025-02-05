#!/usr/bin/env python3
import unittest
import numpy as np
import os, sys

_VERSION_DIR = os.path.abspath(os.path.dirname(__file__))
_VERSIONS = []
for file in os.scandir(_VERSION_DIR):
    if file.is_file() and file.name.startswith('test_R') and file.name.endswith('.ctf'):
        _VERSIONS.append({'file': os.path.join(_VERSION_DIR, file.name),
                          'version': file.name.split('test_')[1].split('.')[0]
                          })

class PacePythonTest(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        import libpymcr
        for ver in _VERSIONS:
            mlPath = libpymcr.utils.checkPath(ver['version'], error_if_not_found=False)
            if mlPath is not None:
                cls.m = libpymcr.Matlab(ver['file'], mlPath)
                print(f'Using Matlab version {ver["version"]}')
                return
        raise RuntimeError('Could not find a valid Matlab version')

    @classmethod
    def tearDownClass(cls):
        with open('success', 'w') as f:
            f.write('success')
    
    def test_numpy_basics(self):
        # Tests some basic functions
        magic = self.m.magic(3)
        self.assertIs(type(magic), np.ndarray)
        self.assertEqual(magic.shape, (3, 3))
        self.assertTrue(np.allclose(magic, np.array([[8, 1, 6], [3, 5, 7], [4, 9, 2]])))
        # Tests that Matlab and numpy give same eigenvalues for symmetric real matrix
        mat = np.random.rand(4, 4)
        mat = np.triu(mat) + np.triu(mat, 1).T
        v0, e0 = self.m.eig(mat)
        e1, v1 = np.linalg.eig(mat)
        idx = np.argsort(e1)
        self.assertTrue(np.allclose(np.diag(e0), e1[idx]))
        self.assertTrue(np.allclose(np.abs(v0), np.abs(v1[:, idx])))

    def test_numpy_wrap(self):
        # Tests data array wrapping works
        # We do this by doing a passing a numpy array through a Matlab identity function
        f_iden = self.m.str2func('@(x) x')
        v_np = np.random.rand(1, 2000)
        v_mat = self.m.feval(f_iden, v_np)
        # v_np and v_mat should be the _same_ matrix (it's just passed straight through)
        self.assertEqual(id(v_np), id(v_mat))
        self.assertEqual(sys.getrefcount(v_np), 3)
        # Now try with a matrix
        m_np = np.random.rand(40, 40)
        m_mat = self.m.feval(f_iden, m_np)
        # Unfortunately, although Matlab accepts row-major arrays, on return from most
        # functions it force converts to column major. Thus m_mat is actually a wrapped
        # _Matlab_ array (note we can't actually create a matlab_wrapper in Python so
        # cannot use isinstance so must use the string as comparison)
        self.assertIn('libpymcr.matlab_wrapper', str(type(m_mat.base)))
        self.assertTrue(np.allclose(m_np, m_mat))
        # Now try again but with a column-wise array.
        mf_np = np.asfortranarray(m_np)
        mf_mat = self.m.feval(f_iden, mf_np)
        self.assertEqual(id(mf_np), id(mf_mat))
        self.assertEqual(sys.getrefcount(mf_np), 3)

    def test_numpy_nd(self):
        # Tests that ND (N>2) conversions are ok.
        m1, m2 = (np.random.rand(11, 11, 11), np.random.rand(11, 11, 11))
        res_np = np.einsum('ijk,jkl', m1, m2)
        res_mat = self.m.mtimes(self.m.reshape(m1, 11, 121),  self.m.reshape(m2, 121, 11))
        self.assertTrue(np.allclose(res_np, res_mat))
        # TODO - wrapped arrays not reshaping correctly
        #n1, n2 = (np.asfortranarray(m1), np.asfortranarray(m2))
        #res_mat_f = self.m.mtimes(self.m.reshape(n1, 11, 121),  self.m.reshape(n2, 121, 11))
        #self.assertTrue(np.allclose(res_np, res_mat_f))

    def test_nested_list_to_array(self):
        # Tests that nested lists are converted correctly to Matlab arrays
        f_iden = self.m.str2func('@(x) x')
        m9 = [[1, 2, 3], [4, 5, 6], [7, 8, 9]]
        c9 = self.m.feval(f_iden, m9)
        # Numeric arrays always converted to double in Matlab (libpymcr "feature")
        self.assertTrue(np.allclose(c9, np.array(m9, dtype=np.double)))
        # Checks N-D arrays correctly converted
        mm = np.random.rand(2,3,4)
        cm = self.m.feval(f_iden, mm)
        self.assertTrue(np.allclose(mm, cm))
        # Check that if the shape is not consistent that a list (cell) is returned
        mc = [[1,2,3], [4,5,6], 7]
        cc = self.m.feval(f_iden, mc)
        self.assertIsInstance(cc, list)
        self.assertIs(len(cc), 3)
        self.assertTrue(np.allclose(cc[0], np.array([1, 2, 3], dtype=np.double)))
        self.assertTrue(np.allclose(cc[1], np.array([4, 5, 6], dtype=np.double)))
        self.assertEqual(cc[2], 7)

    def test_tuple_list_to_cell(self):
        # Tests that tuples of lists are converted to cell arrays of numeric arrays
        f_iden = self.m.str2func('@(x) x')
        m1 = ([1, 2, 3], [4, 5, 6])
        c1 = self.m.feval(f_iden, m1)
        self.assertIsInstance(c1, list)
        self.assertEqual(len(c1), len(m1))
        for ii in range(len(m1)):
            self.assertTrue(np.allclose(c1[ii], np.array(m1[ii], dtype=np.double)))


if __name__ == '__main__':
    unittest.main()
