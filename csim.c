//Author: Daniel Vennemeyer, vennemdp, M14932765


#include "lab4.h"
#define _GNU_SOURCE
#include <math.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>




struct block{
  bool valid;
  int tag;
  int data;
};




struct cache{
  struct block ** sets; //2D array of blocks




  //book-keeping variables
  int hits;
  int misses;
  int evictions;
};








void load(struct cache *myCache, long long address, int s,int E, int b, bool verbose){
  //determine offset
  int offset_bit_mask = 0;
  for (int i = 0; i < b; i++){
      offset_bit_mask = offset_bit_mask << 1;
      offset_bit_mask += 1;
  }
  int offset = address & offset_bit_mask;
  if (verbose) printf("offset: %x \n", offset);




  //determine set index
  int set_bit_mask = 0;
  for (int i = 0; i < s; i++){
      set_bit_mask = set_bit_mask << 1;
      set_bit_mask += 1;
  }
  set_bit_mask = set_bit_mask << b;
  int set_index = (address & set_bit_mask) >> b;
  if (verbose) printf("set index: %x \n", set_index);




  //deterine tag;
  int tag = address >> (b + s);
  if (verbose) printf("tag: %x \n", tag);




  //look in the cache
  int num_valid = 0;
  bool hit = false;
  struct block * open_block;
  for (int i = 0; i < E; i++) //traverse blocks in set
  {
      if (myCache->sets[set_index][i].valid){
          num_valid++;
          if (myCache->sets[set_index][i].tag == tag){ //check whether item is a hit
               //update data in all blocks to reflect use for LRU algo
               for (int j = 0; j < E; j++){
                   myCache->sets[set_index][i].data++;
               }
               myCache->sets[set_index][i].data = 1;


              myCache->hits++; //update hits variables
              hit = true;
              if (verbose) printf("======= Hit =========");
              break;
          }
      }
      else{
          open_block = &myCache->sets[set_index][i]; //pointer to the last unused block (useful if item is missing)
      }
  }




  if (!hit){ //deal with misses/evictions
      if (num_valid == E){ //check if all blocks in set are full
          struct block * LRU_block = &myCache->sets[set_index][0];
          for (int i = 0; i < E; i++) //find LRU block
          {
              if (myCache->sets[set_index][i].data > LRU_block->data){
                  LRU_block = &myCache->sets[set_index][i];
              }
          }
          //pull desired block to LRU block
          LRU_block->tag = tag;
          LRU_block->valid = true;




          myCache->evictions++; //update evictions variable
          if (verbose) printf("======= Evicted =========");
      }
      else{
          //pull desired block to open spot
          open_block->tag = tag;
          open_block->valid = true;
          if (verbose) printf("======= Missed =========");
      }
      myCache->misses++; //update misses variable
  }
  if (verbose) printf("\n");
}








int main(int argc, char** argv)
{
  //intialize CLI arguements
  int v = false;
  int s,E,b = 0;
  FILE* trace;




  //Get CLI arguments
  int opt;
  while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1)
      switch (opt)
      {
      case 'h': //help
           printf(" -h: Optional help flag that prints usage info\n• -v: Optional verbose flag that displays trace info\n• -s <s>: Number of set index bits (S = 2s is the number of sets)\n• -E <E>: Associativity (number of lines per set)\n• -b <b>: Number of block bits (B = 2b is the block size)\n• -t <tracefile>: Name of the valgrind trace to replay\n");
          //return 0;
          break;
      case 'v':
          v = true; //verbose
          break;
      case 's': //num sets
          s = atoi(optarg);
          break;
      case 'E': //num lines
          E = atoi(optarg);
          break;
      case 'b': //block size
          b = atoi(optarg);
          break;
      case 't':
          trace = fopen(optarg, "r");
          if (v) printf("%s", optarg);
          if (trace == NULL)
          {
              printf("ERROR: No such file or directory\n");
              exit(1);
          }
          break;
      default:
          exit(1);
          break;
      }
    




  //initialize cache
  struct cache myCache;
  myCache.misses = 0;
  myCache.evictions = 0;
  myCache.hits = 0;
  myCache.sets = (struct block **)malloc(sizeof(struct block *) * pow(2,s));
  for (int i = 0; i < pow(2,s); i ++){
      myCache.sets[i] = (struct block *)malloc(sizeof(struct block)* E);
      for (int j = 0; j < E; j++){
          myCache.sets[i][j].valid = 0;
          myCache.sets[i][j].tag = 0;
          myCache.sets[i][j].data = 0;
      }
  }




  //intialize variables for tokens in trace file
  char* mode;
  long long address;
  char* size;
   //initialize getline variables
  char * line;
  size_t len = 0;
  ssize_t read;




  while((read = getline(&line, &len, trace))!= -1){ //traverse trace file line by line
      if(line[0] == ' '){ //parse through trace file
          mode = strtok(line, " ");
          address = strtoull(strtok(NULL, ","), NULL, 16);
          size = strtok(NULL, ",");


          if (v) printf("mode: %s\naddress: %llx, size: %s", mode, address, size);


          if (strcmp(mode, "M") == 0){ //perform cache action specified by trace file
              load(&myCache, address, s,E,b, v);
          }
          load(&myCache, address, s,E,b, v);
      }
  }


  printSummary(myCache.hits, myCache.misses, myCache.evictions); //print final output


  for (int i = 0; i < pow(2,s); i ++){ //free malloced elements to prevent memory leaks
       free(&myCache.sets[i][0]);
  }


  return 0;
}



