#ifndef SYMNMF_H_
#define SYMNMF_H_

struct cord {
    double value;
    struct cord *next;
};

struct vector {
    struct vector *next;
    struct cord *cords;
};

double Euclidean_distance(struct cord *cord1, struct cord *cord2);
int count_vectors(struct vector* head);
double** sym(struct vector *head, int n);
double** ddg(double **A, int n);
double** norm(double** A, double** D, int n);
void Free_Matrix(double **matrix, int n);
void Free_vector(struct vector *head);

#endif