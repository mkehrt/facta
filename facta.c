#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Basic syntactic classes.

typedef struct facta_header {
  int ref_count;
  void (*retain)(struct facta_header*);
  void (*release)(struct facta_header*);
  void (*destroy)(struct facta_header*);
} facta_header;

typedef struct facta_type {
  facta_header header;
  char*(*print)(struct facta_type*);
  void(*destroy)(struct facta_type*);
} facta_type;

typedef struct facta_context {
  facta_header header;
  char* name;
  facta_type* ty;
  struct facta_context* next;
} facta_context;

typedef struct facta_environment {
  facta_header header;
  char* name;
  struct facta_expression* value;
  struct facta_environment* next;
} facta_environment;

typedef struct facta_expression {
  facta_header header;
  facta_type* (*typecheck)(facta_context*, struct facta_expression*);
  struct facta_expression_s* (*evaluate)(facta_environment*, struct facta_expression*);
  char* (*print)(struct facta_expression*);
} facta_expression;

void facta_noop( /* anything */ ) {
}

void facta_retain(facta_header* header) {
  header->ref_count++;
  if (header->ref_count == 0) {
    header->destroy(header);
  }
}

void facta_release(facta_header* header) {
  header->ref_count--;
}

// Types

// Int

typedef struct facta_type_int {
  facta_type type_header;
} facta_type_int;


char* facta_type_int_print(facta_type* ty) {
  return "int";
}

void facta_type_int_destroy(facta_type* ty) {
}

facta_type_int facta_type_int_static = {
  {
    {
      1,
      facta_noop,
      facta_noop,
      facta_noop
    },
    facta_type_int_print,
    facta_type_int_destroy
  }
};

facta_type* facta_type_int_create(void) {
  return (facta_type*)&facta_type_int_static;
}

// Arrow

typedef struct facta_type_arrow {
  facta_type type_header;
  facta_type* t1;
  facta_type* t2;
} facta_type_arrow;

char* facta_type_arrow_print(facta_type* type_header) {
  facta_type_arrow* ty = (facta_type_arrow*) type_header;
  char* left = ty->t1->print(ty->t1);
  char* right = ty->t2->print(ty->t2);
  size_t length_left = strlen(left);
  size_t length_right = strlen(right);
  size_t size = 2 + length_left + 4 + length_right + 2 + 1;
  char* ret = (char*)malloc(size);
  sprintf(ret, "( %s -> %s )", left, right);
  free(left);
  free(right);
  return ret;
}

void facta_type_arrow_destroy(facta_header* header) {
  facta_type_arrow* ty = (facta_type_arrow*)header;
  ty->t1->header.release(&ty->t1->header);
  ty->t2->header.release(&ty->t2->header);
  free(ty);
}

facta_type* facta_type_arrow_create(facta_type* left, facta_type* right) {
  left->header.retain(&left->header);
  right->header.retain(&right->header);

  facta_type_arrow* ty = (facta_type_arrow*)sizeof(facta_type_arrow);

  ty->type_header.header.ref_count = 1;
  ty->type_header.header.retain = facta_retain;
  ty->type_header.header.release = facta_release;
  ty->type_header.header.destroy = facta_type_arrow_destroy;

  ty->type_header.print=facta_type_arrow_print;

  ty->t1 = left;
  ty->t2 = right;

  return (facta_type*)ty;
}

int main() {
  facta_type* i1 = facta_type_int_create();
  facta_type* i2 = facta_type_int_create();

  facta_type* arr = facta_type_arrow_create(i1, i2);

  i1->header.release(&i1->header);
  i2->header.release(&i1->header);

  char* s = arr->print(arr);
  printf("%s", s);
  free(s);

  arr->header.release(&arr->header);

  return 0;
}
