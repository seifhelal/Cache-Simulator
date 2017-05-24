//
//  main.cpp
//  Assembly Project 2
//
//  Created by SEIF on 5/13/17.
//  Copyright © 2017 SEIF. All rights reserved.
//

#include <iostream>
#include <string>
#include <vector>
#include <iomanip> 
#include <cmath>
#include <cstdlib>
#include <ctime>

using namespace std;
int cash_type, block_size, cash_size,number_of_blocks=0;;

int compulsry_misses=0, capcity_misses=0, conflict_misses=0;


#define		DRAM_SIZE		(64*1024*1024)

unsigned int m_w = 0xABABAB55;    /* must not be zero, nor 0x464fffff */
unsigned int m_z = 0x05080902;    /* must not be zero, nor 0x9068ffff */




unsigned int rand_()
{
    m_z = 36969 * (m_z & 65535) + (m_z >> 16);
    m_w = 18000 * (m_w & 65535) + (m_w >> 16);
    return (m_z << 16) + m_w;  /* 32-bit result */
}

unsigned int memGen1()
{
    static unsigned int addr=0;
    return (addr++)%(DRAM_SIZE);
}

unsigned int memGen2()
{
    static unsigned int addr=0;
    return  rand_()%(128*1024);
}

unsigned int memGen3()
{
    return rand_()%(DRAM_SIZE);
}

unsigned int memGen4()
{
    static unsigned int addr=0;
    return (addr++)%(1024);
}

unsigned int memGen5()
{
    static unsigned int addr=0;
    return (addr++)%(1024*64);
}

unsigned int memGen6()
{
    static unsigned int addr=0;
    return (addr+=256)%(DRAM_SIZE);
}


// Cache Simulator
bool cacheSim(unsigned int addr, int cash[3][100000], int type, int &block_counter, int index_addr, int tag_addr)
{
    int shift_offset=log2(block_size);
    bool detected=false;
    bool misses_flag=true;
    
    if (cash_type==0)    // Direct Mapped ******************************************
    {
        if (cash[0][index_addr]==tag_addr)
        {
            return true;
        }
        else
            {
                cash[0][index_addr]= tag_addr;
                for (int i=0; i < number_of_blocks; i++)
                {
                    if (cash[1][i]!=1)
                    {   misses_flag=false;
                        i=number_of_blocks;}
                    
                }
                //calculating misses
                if (misses_flag)
                    capcity_misses++;   // capcity miss because the cash is full
                else
                {
                    if(cash[1][index_addr]==1)
                        conflict_misses++;
                    else
                    {
                        compulsry_misses++;
                    }
                }
                cash[1][index_addr]= 1;
                return 0;
            }
    } // end of directed mapped
    ///////////////////////////////////////////////////////////////////
    
    else if (cash_type==1)   // set asscoiative *************************************
    {
        index_addr=index_addr * type;
        for (int i=0; i < type ; i++)
        {
            if (cash[0][index_addr+i]==tag_addr)
            {
               // cout <<"0x hitttttttt" << setfill('0') << setw(8) << hex << tag_addr << endl;
                return 1;
            }
        }
        for (int j=0; j < type; j++)
        {
            if (cash[1][index_addr+j] == -1)
            {
                compulsry_misses++;
                cash[0][index_addr+j]=tag_addr;
                cash[1][index_addr+j]=1;
                return 0;
            }
        }
        
        srand(time(NULL));
        int x=rand()%(type);
        cash[0][index_addr+x]=tag_addr;
        cash[1][index_addr+x]=1;
        capcity_misses++;
        return 0;
        
    }//end of set assciative

    
    
    else if (cash_type==2)        // fully associative **************************************
    {
        if (type==0)    // LRU /////////
        {
            if (block_counter < number_of_blocks)
            {
                for (int i=0; i < number_of_blocks; i++)
                {
                    if (cash[0][i]==addr >> shift_offset)
                    {
                        detected=true;
                        cash[1][i]=block_counter;
                        block_counter--;
                        return detected; //hit
                    }
                }
                
                if (!detected)
                {
                    compulsry_misses++;
                    cash[0][block_counter]=addr>>shift_offset;
                    cash[1][block_counter]=block_counter;
                    return false;  //miss
                }
            }
            else  // block counter is more than block size
            {
                //check for existence
                for (int i=0; i < number_of_blocks; i++)
                {
                    if (cash[0][i]==(addr >> shift_offset))
                    {
                        detected=true;
                        cash[1][i]=block_counter;
                        //block_counter--;
                        return detected; //hit
                    }
                }
                
                if (!detected)
                {
                    int compare=0;
                    for (int i=1; i < number_of_blocks; i++)
                    {
                        if (cash[1][compare] > cash[1][i])
                            compare=i;
                    }
                    cash[0][compare]=addr >> shift_offset;
                    cash[1][compare]=block_counter;
                    capcity_misses++;
                    return false; //miss

                }
            }
        }   // end of LRU
        
        else if (type==1)   // LFU ///////////////
        {
            if (block_counter < number_of_blocks)
            {
                for (int i=0; i < number_of_blocks; i++)
                {
                    if (cash[0][i]==addr >> shift_offset)
                    {
                        detected=true;
                        cash[1][i]=cash[1][i]+1;
                        block_counter--;
                        return detected; //hit
                    }
                }
                
                if (!detected)
                {
                    cash[0][block_counter]=addr>>shift_offset;
                    cash[1][block_counter]=-1;
                    compulsry_misses++;
                    return false;  //miss
                }
            }
            else  // block counter is more than block size
            {
                //check for existence
                for (int i=0; i < number_of_blocks; i++)
                {
                    if (cash[0][i]==(addr >> shift_offset))
                    {
                        detected=true;
                        cash[1][i]++;
                        block_counter--;
                        return detected; //hit
                    }
                }
                if (!detected)
                {
                    int compare2=0;
                    for (int i=1; i < number_of_blocks; i++)
                    {
                        if (cash[1][compare2] >= cash[1][i])
                            compare2=i;
                    }
                    cash[0][compare2]=addr >> shift_offset;
                    cash[1][compare2]=-1;
                    capcity_misses++;
                    return false; //miss
                }
            }

        }  // end if LFU
        
        else if (type==2)
        {
            if (block_counter < number_of_blocks)
            {
                for (int i=0; i < number_of_blocks; i++)
                {
                    if (cash[0][i]==addr >> shift_offset)
                    {
                        detected=true;
                        block_counter--;
                        return detected; //hit
                    }
                }
                
                if (!detected)
                {
                    cash[0][block_counter]=addr>>shift_offset;
                    cash[1][block_counter]=block_counter;
                    compulsry_misses++;
                    return false;  //miss
                }
            }
            else  // block counter is more than block size
            {
                //check for existence
                for (int i=0; i < number_of_blocks; i++)
                {
                    if (cash[0][i]==(addr >> shift_offset))
                    {
                        detected=true;
                        //block_counter--;
                        return detected; //hit
                    }
                }
                
                if (!detected)
                {
                    int compare=0;
                    for (int i=1; i < number_of_blocks; i++)
                    {
                        if (cash[1][compare] > cash[1][i])
                            compare=i;
                    }
                    cash[0][compare]=addr >> shift_offset;
                    cash[1][compare]=block_counter;
                    capcity_misses++;
                    return false; //miss
                }
            }
        }// end of FIFO
        else if (type==3)
        {
            if (block_counter < number_of_blocks)
            {
                for (int i=0; i < number_of_blocks; i++)
                {
                    if (cash[0][i]==addr >> shift_offset)
                    {
                        detected=true;
                        block_counter--;
                        return detected; //hit
                    }
                }
                
                if (!detected)
                {
                    cash[0][block_counter]=addr>>shift_offset;
                    compulsry_misses++;
                    return false;  //miss
                }
            }
            else  // block counter is more than block size
            {
                //check for existence
                for (int i=0; i < number_of_blocks; i++)
                {
                    if (cash[0][i]==(addr >> shift_offset))
                    {
                        detected=true;
                        //block_counter--;
                        return detected; //hit
                    }
                }
                
                if (!detected)
                {
                    srand(time(NULL));
                    cash[0][rand()%number_of_blocks]=addr >> shift_offset;
                    capcity_misses++;
                    return 0; //miss
                }
            }
        }//end of RANDOM
    }  // end of Fully associative
    
    
    
    return true;
}

char *msg[2] = {"Miss","Hit"};


///////////////////////////////////////////////////////////////////

int main(int argc, const char * argv[]) {
    
    int looper=1000000, addr, flag, shift;

    cout << "Please, enter 0 for Direct mapped, 1 for set associative, 2 for fully associative: " << endl;
    cin >> cash_type;
    cout << "Please, enter the size of the block as a Power of 2 between 4 and 128 byte  :" << endl;
    cin >> block_size;
    cout << "Please, enter cache size: 1KB – 64KB; in steps that are power of 2: " << endl;
    cin >> cash_size;
    
    

    int cash[3][100000];
    int block_counter=0;
    int hit_counter=0;
    int index_addr=0, tag_addr=0;
    
    ///////////////////////////////////////////////////////////////////
    if ( cash_type==0)   //Direct_mapped **************
    {
        number_of_blocks= (cash_size*1024)/block_size;
        
        ////////////////////
        for (int i=0; i < 2; i++)   // setting all the cash with -1
            for (int j=0; j < number_of_blocks; j++)
                cash[i][j]=-1;
        //////////////////
        
        for(int i=0; i <looper ;i++)
        {
            addr = memGen1();
            shift= log2(block_size);
            index_addr= (addr >> shift)% number_of_blocks;
            shift= log2(number_of_blocks+block_size);
            tag_addr= addr >>shift;    // shifted the amount the offset and the index sizes
            flag = cacheSim(addr, cash, 0,block_counter, index_addr, tag_addr);
            index_addr=0;
            tag_addr=0;
            cout <<"0x" << setfill('0') << setw(8) << hex << addr <<" ("<< msg[flag] <<")\n";
            if (msg[flag]=="Hit")
            {
                hit_counter++;
            }
        }
    cout << "Hits  " << hit_counter<<endl << "Compulsry:  " << compulsry_misses <<endl<< "Capcity:  " << capcity_misses <<endl<< "Conflict:  " << conflict_misses <<  endl;

    }
    ///////////////////////////////////////////////////////////////////
    else if (cash_type==2)  //  Fully associative**************
    {
        int replacment_type;
        number_of_blocks= (cash_size*1024)/block_size;
        
        cout << "please, enter the type of replacment for the Fully Associative: LRU->0 , LFU->1, FIFO->2, RANDOM->3  :- " << endl;
        cin >> replacment_type;
        
        
        for (int i=0; i < 2; i++)   // setting all the cash with -1
            for (int j=0; j < number_of_blocks; j++)
                cash[i][j]=-10;
        
        for(int i=0; i <looper ;i++)
        {
            addr = memGen4();
            flag = cacheSim(addr, cash, replacment_type, block_counter, index_addr, tag_addr);
           // cout <<"0x" << setfill('0') << setw(8) << hex << addr <<" ("<< msg[flag] <<")\n";
            
            if (msg[flag]=="Hit")
            {
                hit_counter++;
            }
            block_counter++;

        }
        
        cout << "Hits  " << hit_counter<<endl << "Compulsry:  " << compulsry_misses <<endl<< "Capcity:  " << capcity_misses <<endl<< "Conflict:  " << conflict_misses <<  endl;
        
    } // end of fully associative main
    else if (cash_type==1) // set associative
    {
        int number_of_ways;
        cout << "please, enter the number of ways for the set associative cash: 2,4,8,16" << endl;
        cin >> number_of_ways;
        number_of_blocks= (cash_size*1024)/(block_size*number_of_ways);
        
        for (int i=0; i < 3; i++)   // setting all the cash with -1
            for (int j=0; j < 100000; j++)
                cash[i][j]=-1;
        
        for(int i=0; i <looper ;i++)
        {
            addr = memGen5();
            shift= log2(block_size);
            index_addr= (addr >> shift)% (number_of_blocks);
            shift= log2(number_of_blocks+block_size);
            tag_addr= addr >>shift;
            flag = cacheSim(addr, cash, number_of_ways, block_counter, index_addr, tag_addr);
            // cout <<"0x" << setfill('0') << setw(8) << hex << addr <<" ("<< msg[flag] <<")\n";
            index_addr=0;
            tag_addr=0;
            if (msg[flag]=="Hit")
            {
                hit_counter++;
            }
            block_counter++;
        }
        
        cout << "Hits  " << hit_counter<<endl << "Compulsry:  " << compulsry_misses <<endl<< "Capcity:  " << capcity_misses <<endl<< "Conflict:  " << conflict_misses <<  endl;
    }
  
}


