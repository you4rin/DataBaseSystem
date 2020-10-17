#include "../include/defines.h"
#include "../include/bpt.h"
#include "../include/file.h"

// MAIN

int main(int argc, char** argv) {

    char* input_file;
    char output_file[2000];
    FILE* fp;
    node* root;
    page_t* header;
    int input, range2;
    key_type key;
    char* value = (char*)malloc(sizeof(char)*120);
    char instruction;
    char license_part;

    root = NULL;
    verbose_output = false;

    if (argc > 2) {
        leaf_order = atoi(argv[1]);
        internal_order = atoi(argv[2]);
    }
    
    get_path();
    scanf("%s", output_file);
    getchar();
    open_table(output_file);
    license_notice();
    usage_1();  
    usage_2();
    
    if (argc > 3) {
        input_file = argv[3];
        fp = fopen(input_file, "r");
        if (fp == NULL) {
            perror("Failure  open input file.");
            exit(EXIT_FAILURE);
        }
        while (1) {
            fscanf(fp, "%lld ", &key);
            if(feof(fp))break;
            fgets(value, 120, fp);
            db_insert(key, value);
            if(feof(fp))break;
        }
        fclose(fp);
        //print_tree(root);
    }
    
    if(argc > 4){
        input_file = argv[4];
        fp = fopen(input_file, "r");    
        if (fp == NULL) {
            perror("Failure  open input file.");
            exit(EXIT_FAILURE);
        }
        while(1){
            fscanf(fp, "%lld\n", &key);
            if(db_find(key, value)){
                record_not_found(key);
                break;
            }
            else
                print_result(key, value);
            if(feof(fp))break;
        }
        fclose(fp);
    }
    
    if(argc > 5){
        input_file = argv[5];
        fp = fopen(input_file, "r");
        if (fp == NULL) {
            perror("Failure  open input file.");
            exit(EXIT_FAILURE);
        }
        while(1){
            fscanf(fp, "%lld\n", &key);
            if(db_delete(key))break;
            if(feof(fp))break;
        }
        fclose(fp);
        input_file = argv[4];
        fp = fopen(input_file, "r");    
        if (fp == NULL) {
            perror("Failure open input file.");
            exit(EXIT_FAILURE);
        }
        while(1){
            fscanf(fp, "%lld\n", &key);
            if(db_find(key, value))
                record_not_found(key);
            else{
                print_result(key, value);
                break;
            }
            if(feof(fp))break;
        }
        
        input_file = argv[3];
        fp = fopen(input_file, "r");
        if (fp == NULL) {
            perror("Failure  open input file.");
            exit(EXIT_FAILURE);
        }
        while (1) {
            fscanf(fp, "%lld ", &key);
            if(feof(fp))break;
            fgets(value, 120, fp);
            db_insert(key, value);
            if(feof(fp))break;
        }
        fclose(fp);
        
    }

    printf("> ");
    while (scanf("%c", &instruction) != EOF) {
        switch (instruction) {
        case 'd':
            scanf("%lld", &key);
            db_delete(key);
            //print_tree(root);
            break;
        case 'i':
            scanf("%lld %120[^\n]", &key, value);
            db_insert(key, value);
            //print_tree(root);
            break;
        case 'f':
        //case 'p':
            scanf("%lld", &key);
            if(db_find(key, value))
                record_not_found(key);
            else{
                print_result(key, value);
            }
            break;
        /*
        case 'r':
            scanf("%d %d", &input, &range2);
            if (input > range2) {
                int tmp = range2;
                range2 = input;
                input = tmp;
            }
            find_and_print_range(root, input, range2, instruction == 'p');
            break;
        case 'l':
            print_leaves(root);
            break;
        */
        case 'q':
            while (getchar() != (int)'\n');
            free(value);
            return EXIT_SUCCESS;
            break;
        /*
        case 't':
            print_tree(root);
            break;
        case 'v':
            verbose_output = !verbose_output;
            break;
        case 'x':
            if (root)
                root = destroy_tree(root);
            print_tree(root);
            break;
        */
        default:
            usage_2();
            break;
        }
        while (getchar() != (int)'\n');
        printf("> ");
    }
    printf("\n");

    free(value);

    return EXIT_SUCCESS;
}
