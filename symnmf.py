import numpy as np
import pandas as pd
import math
import sys
import symnmf_capi as symnmf

np.random.seed(1234)

def init_H(vectors : np.ndarray, k : int):
    m = np.sum(np.sum(vectors, axis=0), axis=0) / (vectors.shape[0] * vectors.shape[1])
    H = np.random.uniform(0, 2*math.sqrt(m/k), size=(vectors.shape[0], k))
    return H

def update_H(H : np.ndarray, W: np.ndarray, max_iter : int):
    beta = 0.5
    for n in range(max_iter):
        new_H = np.zeros(H.shape)
        numerator_table = np.matmul(W, H)
        denominator_table = np.matmul(H, H.T)
        denominator_table = np.matmul(denominator_table, H)
        
        for i in range(H.shape[0]):
            for j in range(H.shape[1]):
                numerator = numerator_table[i][j]
                denominator = denominator_table[i][j]
                
                new_H[i][j] = H[i][j] * (1 - beta * (numerator / denominator))
        
        conv_table = new_H - H
        conv_val = np.linalg.norm(conv_table, ord="fro", axis=None, keepdims=False)
        conv_val = conv_val ** 2
        H = new_H
        if conv_val < math.exp(-4):
            break
        
    return H

def main():
    if len(sys.argv) != 4:
        print("Usage: python script.py <k> <goal> <file_name>")
        sys.exit(1)
    
    k = int(sys.argv[1])
    goal = sys.argv[2]
    file_name = sys.argv[3]

    # Read the data file
    df = pd.read_csv("demo.txt", header=None)
    print("Data:")
    print(df)
    print()
    
    # Read as NumPy array
    vectors = df.to_numpy()                    # NumPy array for init_H()

    # Convert to list for C API  
    vectors_list = vectors.tolist()            # Python list for symnmf.norm()

    # Call C API
    W = symnmf.norm(vectors_list, vectors.shape[0])  # Returns Python list

    # Convert back to NumPy array
    W = np.array(W)  
    
    # Initialize H matrix
    H = init_H(vectors, k)
    print("Initial H:")
    print(H)
    print()
    
    # Update H matrix
    final = update_H(H, W, 300)
    print("Final H:")
    print(final)

if __name__ == "__main__":
    main()