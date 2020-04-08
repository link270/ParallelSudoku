#include <algorithm>
#include <iostream>
#include <mpi.h>
#include <math.h>
#include <vector>

#define MCW MPI_COMM_WORLD

#define N 9
#define box 3

void fillBox(std::vector<int>& puzzle);
std::vector<int> generatePuzzle(bool basic);
int getBox(int puzzleIndex);
int getColumn(int puzzleIndex);
int getRow(int puzzleIndex);
bool isValid(std::vector<int> puzzle);
void printPuzzle(std::vector<int> puzzle);
void solvePuzzle(std::vector<int> puzzle);

void fillBox(std::vector<int>& puzzle, int boxNum){
    std::vector<int> values;
    for(int i=0;i<N;++i){
        values.push_back(i);
    }

    std::random_shuffle(values.begin(), values.end());
    int i=0;
    while(!values.empty()){
        if(getBox(i)==boxNum){
            puzzle[i]=values.back();
            values.pop_back();
        }
        ++i;
    }
}

std::vector<int> generatePuzzle(bool basic){
    std::vector<int> puzzle(N*N, -1);
    if(basic){
        // This isn't super elegant, but we can use it for the time being
        // to have a basic puzzle to test for consistancy, instead of random ones each time.
        //  =====================
        // ||3|*|*||*|*|9||1|7|8||
        // ||*|*|6||1|5|*||9|4|*||
        // ||*|9|1||3|*|*||6|5|*||
        //  =====================
        // ||9|*|*||*|*|5||*|2|*||
        // ||6|*|*||2|4|*||5|*|*||
        // ||*|*|4||*|*|*||3|*|*||
        //  =====================
        // ||*|7|*||*|8|6||2|*|*||
        // ||1|6|*||4|*|7||8|9|*||
        // ||5|*|*||*|*|2||4|*|7||
        //  =====================
        puzzle[0] = 3;
        puzzle[5] = 9;
        puzzle[6] = 1;
        puzzle[7] = 7;
        puzzle[8] = 8;
        puzzle[11] = 6;
        puzzle[12] = 1;
        puzzle[13] = 5;
        puzzle[15] = 9;
        puzzle[16] = 4;
        puzzle[19] = 9;
        puzzle[20] = 1;
        puzzle[21] = 3;
        puzzle[24] = 6;
        puzzle[25] = 5;
        puzzle[27] = 9;
        puzzle[32] = 5;
        puzzle[34] = 2;
        puzzle[36] = 6;
        puzzle[39] = 2;
        puzzle[40] = 4;
        puzzle[42] = 5;
        puzzle[47] = 4;
        puzzle[51] = 3;
        puzzle[51] = 3;
        puzzle[55] = 7;
        puzzle[58] = 8;
        puzzle[59] = 6;
        puzzle[60] = 2;
        puzzle[63] = 1;
        puzzle[64] = 6;
        puzzle[66] = 4;
        puzzle[68] = 7;
        puzzle[69] = 8;
        puzzle[70] = 9;
        puzzle[72] = 5;
        puzzle[77] = 2;
        puzzle[78] = 4;
        puzzle[80] = 7;
    }else
    {
        int diagonalSeed = sqrt(N)+1;
        for(int i=0;i<N;++i){
            if(i%diagonalSeed==0) fillBox(puzzle, i);
        }
    }

    return puzzle;
}

// Gives the box number of the puzzle index.
// Box numbers increase from 
int getBox(int puzzleIndex){
    return (puzzleIndex % N) / box + box * (puzzleIndex / (N*box));
}

// Give you the column of the puzzle index
// Columns start at 0-8
int getColumn(int puzzleIndex){
    return puzzleIndex%N;
}

// Give you the row of the puzzle index
// Rows start at 0-8
int getRow(int puzzleIndex){
    return puzzleIndex/N;
}


bool isValid(std::vector<int> puzzle){
    //iterate over all values
    for(int i = 0; i < N*N; ++i){
        //cell not filled, move onto the next one
        if(puzzle[i] == -1)
            continue;
        //Row is all indices in [Nx(i/3), +N]
        //Values in row are found by adding 1, modulus N, then adding to get back to the correct row if necessary
        int row = getRow(i);
        for(int j = (i+1) % N + (N*row); j != i; j = ((++j) % N) + (N*row)){
            if(puzzle[j] == -1)
                continue;
            if(puzzle[i] == puzzle[j]){
                return false;
            }
        }
        
        //Values in column are found by adding/subtracting N
        for(int j = i + N; j != i; j=(j+N) % ( N*N )){
            if(puzzle[j] == -1)
                continue;
            if(puzzle[i] == puzzle[j]){
                return false;
            }
        }
        
        //Values in box are found by finding the minimum and maxiumum possible index that could share a box, and iterating from there.
        int currBox = getBox(i);
        int start = i - (N*box) - box;
        start = (start < 0) ? 0 : start;
        int end = i + (N*box) + box;
        end = (end < N*N) ? end : N*N;
        for(int j = start; j < end; ++j){
            int jBox = getBox(j);
            if(j != i && puzzle[j] != -1 && jBox == currBox){
                if(puzzle[i] == puzzle[j]){
                    return false;
                }
            }
        }
    }
    return true;
}

void printPuzzle(std::vector<int> puzzle){
    int rowLength = N*2+box+(N-9);
    bool overNine = (N-9);
    
    std::cout<<" "<<std::string(rowLength, '=')<<std::endl;
    for(int i=0;i<puzzle.size();){
        for(int j=0;j<box;++j){
            for(int k=0;k<box;++k){
                std::cout<<"|";
                for(int l=0;l<box;++l){
                    if(puzzle[i]==-1) std::cout<<"|*";
                    else std::cout<<((overNine&&puzzle[i]<9) ? "| " : "|")<<puzzle[i];
                    ++i;
                }
            }
            std::cout<<"||"<<std::endl;
        }
        std::cout<<" "<<std::string(rowLength, '=')<<std::endl;
    }
}

//I'll give this a return value when it's closer to finished
void solvePuzzle(std::vector<int> puzzle){
    std::vector<int> queue;
    int start = 0;
    while(puzzle[start] != -1){
        ++start;
    }
    queue.push_back(start);
    int curr = queue.back();
    while(queue.back() < N*N){
        int i = 1;
        while (1){
            if(queue.size() == 0) break;
            puzzle[queue.back()] = i;
            //if(queue.back() == 62){
                //printPuzzle(puzzle);
                //std::cout << "Puzzle is " << ((isValid(puzzle)) ? "valid" : "invalid") << std::endl;
            //}
            if(!isValid(puzzle)){
                ++i;
                while(i > N){
                    puzzle[queue.back()] = -1;

                    queue.pop_back();
                    if(queue.size() > 0)
                        i = puzzle[queue.back()] + 1;
                    else break;
                }
            } else {
                break;
            }
        }

        if(queue.size() > 0)
            curr = queue.back();
        else break;
        do {
            curr++;
        } while (curr < N*N && puzzle[curr] != -1);
        queue.push_back(curr);
    }

    printPuzzle(puzzle);
    std::cout << "Puzzle is " << ((isValid(puzzle)) ? "valid" : "invalid") << std::endl;
    
}

int main(int argc, char **argv){
    int rank, size, data;
    std::vector<int> puzzle;
    

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank); 
    MPI_Comm_size(MCW, &size); 
    srand(rank+time(0));

    if(rank==0){
        puzzle = generatePuzzle(true);
        printPuzzle(puzzle);

        if(!isValid(puzzle)){
            printf("Invalid puzzle\n");
        } else printf("Seems valid\n");
        solvePuzzle(puzzle);
    }

    MPI_Finalize();

    return 0;
}
