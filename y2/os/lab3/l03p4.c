/*
 * The given program manages an in memory array of "student" structures, 
 * applying one of the following operations requested from keyboard by the user:
 *      add     - add a new student
 *          After the command name, the following arguments should be give: firstname, lastname, age, height.
 *          First and last name could not contain spaces.
 *      del     - remove a student (structure), based on its index in the array
 *          The command must receive as an argument the position (i.e. a number) of the structure to be removed
 *      list    - display the entire list of students
 *      exit    - exits from the application
 * When your program starts, the student array is empty.
 */

#include <stdlib.h>
#include <stdio.h>

#define STUDENT_VECTOR_CAPACITY     2
#define STUDENT_MAX_NAME            30
#define MAX_COMMAND                 10

#define TRUE    1
#define FALSE   0

struct student 
{
    char *name;     /* no blanks */
    char *surname;  /* no blanks */
    int age; 
    float height; 
};

struct student_vector
{
    struct student **students;
    int count;
    int capacity;
};

void del_student(struct student* student) {
    free(student->name);
    free(student->surname);
    free(student);
}

struct student_vector *student_vector_create()
{
    struct student_vector *student_vector;

    student_vector = (struct student_vector*)malloc(sizeof(struct student_vector));
    if (NULL == student_vector) {
        return NULL;
    }
    
    student_vector->students = (struct student**)malloc(STUDENT_VECTOR_CAPACITY * sizeof(struct student*));
    if (NULL == student_vector->students) {
        return NULL;
    }
    student_vector->capacity = STUDENT_VECTOR_CAPACITY;
    student_vector->count = 0;
    
    return student_vector;
}

int student_vector_add(struct student_vector *student_vector, struct student *student)
{
    if (student_vector->capacity == student_vector->count) {
        struct student **new_students;
        int i;
        
        new_students = (struct student**)malloc((2 * student_vector->capacity) * sizeof(struct student*));
        if (NULL == new_students) {
            return FALSE;
        }
        for (i = 0; i < student_vector->count; i += 1) {
            new_students[i] = student_vector->students[i];
        }
        student_vector->capacity *= 2;
        free(student_vector->students);
        student_vector->students = new_students;
    }
    student_vector->students[student_vector->count] = student;
    student_vector->count += 1;
    return TRUE;
}

int student_vector_remove(struct student_vector *student_vector, int index)
{
    int i;
    if (index < 0 || index >= student_vector->count) {
        return FALSE;
    }
    student_vector->count -= 1;

    del_student(student_vector->students[index]);

    for (i = index; i < student_vector->count; i += 1) {
        student_vector->students[i] = student_vector->students[i + 1];
    }
    return TRUE;
}

int main()
{
    char *command;
    int number_of_fields_read, i;
    struct student_vector *student_vector;
    int success;
    
    student_vector = student_vector_create();
    if (NULL == student_vector) {
        printf("Not enough memory to run program!\n");
        return 1;
    }
    
    // Am mutat declararea lui command pentru ca imi parea absurd ca la fiecare
    // pas din for sa fie alocata o noua bucata de memorie.
    // Astfel, command trebuie dealocat la finalul programului (inainte de exit),
    // dar in configuratia initiala, command trebuia dealocat ata la finalul
    // programului, cat si la finalul fiecarui pas din for, ceea ce nu-mi pare
    // ca indeplineste "good coding practices" (aceeasi alocare sa poata fi
    // dealocata din 2 locuri separate).
    command = (char*)malloc(MAX_COMMAND + 1);
    
    for (;;) {
        printf("$ ");
        if (NULL == command) {
            printf("ERROR! Out of memory;");
            continue;
        }
        number_of_fields_read = scanf("%10s", command);
        if (1 != number_of_fields_read) {
            printf("ERROR! Cannot process command\n");
            continue;
        }
        // exit
        if ('e' == command[0] && 'x' == command[1] && 'i' == command[2] && 't' == command[3] && '\x00' == command[4]) {
            if (student_vector->count != 0) {
                for (int i = 0; i < student_vector->count; ++i)
                    del_student(student_vector->students[i]);
            }
            free(student_vector->students);
            free(student_vector);
            free(command);
            return 0;
        }
        // list
        if ('l' == command[0] && 'i' == command[1] && 's' == command[2] && 't' == command[3] && '\x00' == command[4]) {
            for (i = 0; i < student_vector->count; i += 1) {
                printf("%3d.%30s%30s%10d%10.1f\n", i, student_vector->students[i]->name, student_vector->students[i]->surname, student_vector->students[i]->age, student_vector->students[i]->height);
            }
        }
        // add
        else if ('a' == command[0] && 'd' == command[1] && 'd' == command[2] && '\x00' == command[3]) {
            struct student *new_student;
            
            new_student = (struct student*)malloc(sizeof(struct student));
            if (NULL == new_student) {
                printf("ERROR! Out of memory!\n");
                continue;
            }
            new_student->name = (char*)malloc(STUDENT_MAX_NAME + 1);
            new_student->surname = (char*)malloc(STUDENT_MAX_NAME + 1);
            if (NULL == new_student->name || NULL == new_student->surname) {
                printf("ERROR! Out of memory!\n");
                continue;
            }
            
            number_of_fields_read = scanf("%30s%30s%d%f", new_student->name, new_student->surname, &new_student->age, &new_student->height);
            if (4 != number_of_fields_read) {
                printf("ERROR! Invalid input!\n");
                continue;
            }
            success = student_vector_add(student_vector, new_student);
            if (success) {
                printf("Student %s %s added successfuly!\n", new_student->name, new_student->surname);
            } else {
                printf("ERROR! Could not add student %s %s!\n", new_student->name, new_student->surname);
            }
        }
        // del
        else if ('d' == command[0] && 'e' == command[1] && 'l' == command[2] && '\x00' == command[3]) {
            number_of_fields_read = scanf("%d", &i);
            if (1 != number_of_fields_read) {
                printf("ERROR! Invalid input!\n");
                continue;
            }
            success = student_vector_remove(student_vector, i);
            if (success) {
                printf("Student %d removed successfully!\n", i);
            } else {
                printf("ERROR! Could not remove student %d!\n", i);
            }
        }
        // unknown command
        else {
            printf("ERROR! Bad command: %s\n", command);
        }
    }
}