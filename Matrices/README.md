# Matrices
## Description
Bash shell scripts to compute matrix operations.
The program is tested with matrices up to dimension 100 x 100.

## Operations:
#### matrix dims [MATRIX]
  - Print the dimensions of the matrix as the number of rows, followed by a space, then the number of columns.
#### matrix transpose [MATRIX]
  - Reflect the elements of the matrix along the main diagonal. Thus, an MxN matrix will become an NxM matrix and the values along the main diagonal will remain unchanged.
#### matrix mean [MATRIX]
  - Take an MxN matrix and return an 1xN row vector, where the first element is the mean of column one, the second element is the mean of column two, and so on.
#### matrix add MATRIX_LEFT MATRIX_RIGHT
  - Take two MxN matrices and add them together element-wise to produce an MxN matrix. add should return an error if the matrices do not have the same dimensions.
#### matrix multiply MATRIX_LEFT MATRIX_RIGHT
  - Take an MxN and NxP matrix and produce an MxP matrix. Note that, unlike addition, matrix multiplication is not commutative. That is A*B != B*A.
