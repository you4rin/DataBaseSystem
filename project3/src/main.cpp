#include <cassert>
#include <ctime>
#include <string.h>
#include <string>
#include <stdint.h>
#include <algorithm>
#include "../include/defines.h"
#include "../include/bpt.h"
#include "../include/db.h"
#include "../include/file.h"
#include "../include/buffer.h"

// MAIN
//int a[10000001];

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
    /*
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
            db_insert(1, key, value);
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
            if(db_find(1, key, value)){
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
            if(db_delete(1, key))break;
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
            if(db_find(1, key, value))
                record_not_found(key);
            else{
                print_result(key, value);
                break;
            }
            if(feof(fp))break;
        }
    }
    */
    //test case #1 & #2: single table, buffer size 100000, 10
    
    init_db(5);
    int tid = open_table("a.db");
    assert(tid == 1);
    std::string str;
    int ret_val;
    for(int i = 1;i < 100001; i++){
        str = std::to_string(i) + " value";
        ret_val = db_insert(tid, i, (char*)str.c_str());
        if(ret_val){
            printf("insertion of key %d failed..\n", i);
            break;
        }
        if(i==100000)
            printf("Insert operation successfully completed\n");
    }
    for(int i = 1; i < 100001; i++){
        ret_val = db_find(tid, i, value);
        if(ret_val){
            printf("finding record with key %d failed..\n", i);
            break;
        }
        if(i==100000)
            printf("Find operation successfully completed\n");
    }
    for(int i = 1; i < 100001; i++){
        ret_val = db_delete(tid, i);
        if(ret_val){
            printf("deletion of key %d failed..\n", i);
            break;
        }
        if(i==100000)
            printf("Delete operation successfully completed\n");
    }
    for(int i = 1; i < 100001; i++){
        ret_val = db_find(tid, i, value);
        if(!ret_val){
            printf("key %d remains\n", i);
            break;
        }
        if(i==100000)
            printf("Find operation successfully completed\n");
    }
    shutdown_db();
    printf("Test completed\n");
    

    //test case #3: two tables, insert 100000, delete 50001~100000
    //close both tables and reopen, and find 1~50000
    /*
    init_db(10000);
    int tid1 = open_table("a.db");
    assert(tid1 == 1);
    int tid2 = open_table("b.db");
    assert(tid2 == 2);
    std::string str;
    int ret_val;
    for(int i = 1; i < 100001; i++){
        str = std::to_string(i) + " value";
        ret_val = db_insert(tid1, i, (char*)str.c_str());
        if(ret_val){
            printf("insertion of key %d in table %d failed..\n", i, tid1);
            break;
        }
        if(i==100000)
            printf("Insert operation in table 1 successfully completed\n");
    }
    for(int i = 1; i < 100001; i++){
        ret_val = db_find(tid1, i, value);
        if(ret_val){
            printf("finding record with key %d in table %d failed..\n", i, tid1);
            break;
        }
        if(i==100000)
            printf("Find operation in table 1 successfully completed\n");
    }
    for(int i = 1; i < 100001; i++){
        str = std::to_string(i) + " value";
        ret_val = db_insert(tid2, i, (char*)str.c_str());
        if(ret_val){
            printf("insertion of key %d in table %d failed..\n", i, tid2);
            break;
        }
        if(i==100000)
            printf("Insert operation in table 2 successfully completed\n");
    }
    for(int i = 1; i < 100001; i++){
        ret_val = db_find(tid1, i, value);
        if(ret_val){
            printf("finding record with key %d in table %d failed..\n", i, tid2);
            break;
        }
        if(i==100000)
            printf("Find operation in table 2 successfully completed\n");
    }
    for(int i = 50001; i < 100001; i++){
        ret_val = db_delete(tid1, i);
        if(ret_val){
            printf("deletion of key %d in table %d failed..\n", i, tid1);
            break;
        }
        if(i==100000)
            printf("Delete operation in table 1 successfully completed\n");
    }
    for(int i = 50001; i < 100001; i++){
        ret_val = db_delete(tid2, i);
        if(ret_val){
            printf("deletion of key %d in table %d failed..\n", i, tid2);
            break;
        }
        if(i==100000)
            printf("Delete operation in table 2 successfully completed\n");
    }
    close_table(tid1);
    close_table(tid2);

    printf("Table closed\n");

    tid1 = open_table("a.db");
    assert(tid1 == 1);
    tid2 = open_table("b.db");
    assert(tid2 == 2);

    printf("Table reopened\n");
    
    for(int i = 1; i < 50002; i++){
        ret_val = db_find(tid1, i, value);
        if(ret_val){
            printf("finding record with key %d in table %d failed..\n", i, tid1);
            break;
        }
    }
    for(int i = 1; i < 50002; i++){
        ret_val = db_find(tid2, i, value);
        if(ret_val){
            printf("finding record with key %d in table %d failed..\n", i, tid2);
            break;
        }
    }

    shutdown_db();
    printf("Test completed\n");
    */
    //test case #4: test for table_size.
    //opening 10 tables, then close all tables, and then open new table
    /*
    init_db(10000);
    int tid[11];
    tid[1] = open_table("a.db");
    assert(tid[1] == 1);
    printf("Opened a.db\n");
    tid[2] = open_table("b.db");
    assert(tid[2] == 2);
    printf("Opened b.db\n");
    tid[3] = open_table("c.db");
    assert(tid[3] == 3);
    printf("Opened c.db\n");
    tid[4] = open_table("d.db");
    assert(tid[4] == 4);
    printf("Opened d.db\n");
    tid[5] = open_table("e.db");
    assert(tid[5] == 5);
    printf("Opened e.db\n");
    tid[6] = open_table("f.db");
    assert(tid[6] == 6);
    printf("Opened f.db\n");
    tid[7] = open_table("g.db");
    assert(tid[7] == 7);
    printf("Opened g.db\n");
    tid[8] = open_table("h.db");
    assert(tid[8] == 8);
    printf("Opened h.db\n");
    tid[9] = open_table("i.db");
    assert(tid[9] == 9);
    printf("Opened i.db\n"); 
    tid[10] = open_table("j.db");
    assert(tid[10] == 10);
    printf("Opened j.db\n");

    close_table(tid[1]);
    close_table(tid[2]);
    close_table(tid[3]);
    close_table(tid[4]);
    close_table(tid[5]);
    close_table(tid[6]);
    close_table(tid[7]);
    close_table(tid[8]);
    close_table(tid[9]);
    close_table(tid[10]);
    printf("Closed all tables\n");
    tid[5] = open_table("e.db");
    assert(tid[5] == 5);
    printf("Reopened e.db\n");
    tid[0] = open_table("k.db");
    assert(tid[0] == -1);
    printf("Failed to open k.db\n");
    printf("Test completed\n");
    */
    
    //test case #5 & #6: check close_table and shutdown_db keeps pinned buffer index
    /*
    printf("Test close_table");
    init_db(10000);
    buf[2].prev = buf_size + 1;
    buf[buf_size + 1].next = 2;
    buf[0].next = buf[0].prev = 1;
    buf[1].next = buf[1].prev = 0;
    buf[1].pin_count = 1;
    buf[1].table_id = 1;
    buf[1].pagenum = (uint64_t)-1;
    buf_cnt++;
    int tid = open_table("a.db");
    assert(tid == 1);
    std::string str;
    int ret_val;
    for(int i = 1; i < 11; i++){
        str = std::to_string(i) + " value";
        ret_val = db_insert(tid, i, (char*)str.c_str());
        if(ret_val){
            printf("insertion of key %d in table %d failed..\n", i, tid);
            break;
        }
    }
    //close_table(tid);
    shutdown_db();
    */
    //test case #7: random inserts & inserting right after deleting data
    /*
    int a[100001], b[100001];
    for(int i=1;i<=100000;i++){
        a[i]=i;
        b[i]=i;
    }
    std::random_shuffle(a+1,a+100001);
    std::random_shuffle(b+1,b+100001);

    init_db(5);
    int tid = open_table("a.db");
    assert(tid == 1);
    std::string str;
    int ret_val;
    for(int i = 1;i < 100001; i++){
        str = std::to_string(i) + " value";
        ret_val = db_insert(tid, a[i], (char*)str.c_str());
        if(ret_val){
            printf("insertion of key %d failed..\n", i);
            break;
        }
        if(i==100000)
            printf("First insert operation successfully completed\n");
    }
    for(int i = 1; i < 100001; i++){
        ret_val = db_find(tid, i, value);
        if(ret_val){
            printf("finding record with key %d failed..\n", i);
            break;
        }
        if(i==100000)
            printf("Find operation successfully completed\n");
    }
    for(int i = 1; i < 100001; i++){
        str = std::to_string(i) + " value";
        ret_val = db_delete(tid, b[i]);
        if(ret_val){
            printf("deletion of key %d failed..\n", i);
            break;
        }
        ret_val = db_insert(tid, b[i], (char*)str.c_str());
        if(ret_val){
            printf("re-insertion of key %d failed..\n", i);
            break;
        }
        if(i==100000)
            printf("Delete and second insert operation successfully completed\n");
    }

    for(int i = 1; i < 100001; i++){
        ret_val = db_find(tid, i, value);
        if(ret_val){
            printf("finding record with key %d failed..\n", i);
            break;
        }
        if(i==100000)
            printf("Find operation successfully completed\n");
    }
    
    shutdown_db();
    printf("Test completed\n");
    */
    //test case #8: opening file after shutdown(cnt should be reset)
    /*
    init_db(100);
    int aid = open_table("a.db");
    assert(aid==1);
    int bid = open_table("b.db");
    assert(bid==2);
    db_insert(aid, 1, "a");
    db_insert(bid, 2, "b");
    shutdown_db();
    init_db(100);
    bid = open_table("b.db");
    //printf("%d\n",bid);
    assert(bid==1);
    int ret_val = db_find(1, 2, value);
    if(ret_val)
        printf("Test failed..\n");
    else
        printf("Test completed\n");
    */
    //test case #9: simple random inserts with large data size
    /*
    for(int i=1;i<=10000000;i++)
        a[i]=i;
    std::random_shuffle(a+1,a+10000001);

    init_db(500000);
    int tid = open_table("a.db");
    assert(tid == 1);
    std::string str;
    int ret_val;
    for(int i = 1;i < 10000001; i++){
        str = std::to_string(i) + " value";
        ret_val = db_insert(tid, a[i], (char*)str.c_str());
        if(ret_val){
            printf("insertion of key %d failed..\n", i);
            break;
        }
        if(i == 10000000)
            printf("Insert operation successfully completed\n");
    }
    for(int i = 1; i < 10000001; i++){
        ret_val = db_find(tid, i, value);
        if(ret_val){
            printf("finding record with key %d failed..\n", i);
            break;
        }
        if(i == 10000000)
            printf("Find operation successfully completed\n");
        //else printf("%d %s\n",i,value);
    }
    for(int i = 1; i < 10000001; i++){
        str = std::to_string(i) + " value";
        ret_val = db_delete(tid, i);
        if(ret_val){
            printf("deletion of key %d failed..\n", i);
            break;
        }
        if(i == 10000000)
            printf("Delete operation successfully completed\n");
    }

    for(int i = 1; i < 10000001; i++){
        ret_val = db_find(tid, i, value);
        if(!ret_val){
            printf("key %d remains..\n", i);
            break;
        }
        if(i == 10000000)
            printf("Find operation successfully completed\n");
        //else printf("%d %s\n",i,value);
    }
    
    shutdown_db();
    */
    //printf("> ");
    //while (scanf("%c", &instruction) != EOF) {
        /*
        switch (instruction) {
        case 'd':
            scanf("%lld", &key);
            db_delete(1, key);
            //print_tree(root);
            break;
        case 'i':
            scanf("%lld %120[^\n]", &key, value);
            db_insert(1, key, value);
            //print_tree(root);
            break;
        case 'f':
        //case 'p':
            scanf("%lld", &key);
            if(db_find(1, key, value))
                record_not_found(key);
            else{
                print_result(key, value);
            }
            break;
        */
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
        /*
        case 'q':
            while (getchar() != (int)'\n');
            free(value);
            return EXIT_SUCCESS;
            break;
        */
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
        /*
        default:
            usage_2();
            break;
        }
        while (getchar() != (int)'\n');
        printf("> ");
        */
    //}
    //printf("\n");
    free(value);

    return EXIT_SUCCESS;
}
