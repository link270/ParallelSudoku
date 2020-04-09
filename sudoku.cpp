#include <algorithm>
#include <iostream>
#include <mpi.h>
#include <math.h>
#include <vector>
#include <random>
#include <iomanip>

#define MCW MPI_COMM_WORLD

#define N 9
#define box 3

void fillBox(std::vector<int>& puzzle);
std::vector<int> generatePuzzle(bool basic, int startingNum);
int getBox(int puzzleIndex);
int getColumn(int puzzleIndex);
int getRow(int puzzleIndex);
bool isValid(std::vector<int> puzzle);
void printPuzzle(std::vector<int> puzzle);
void solvePuzzle(std::vector<int>& puzzle);
void generateQueue(std::vector<std::vector<int>> &queue, std::vector<int> puzzle);

void fillBox(std::vector<int>& puzzle, int boxNum){
    std::vector<int> values;
    for(int i=0;i<N;++i){
        values.push_back(i);
    }

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(values.begin(), values.end(), g);
    int i=0;
    while(!values.empty()){
        if(getBox(i)==boxNum){
            puzzle[i]=values.back()+1;
            values.pop_back();
        }
        ++i;
    }
}

// StartingNum must be 17 or higher to get a unique solution
std::vector<int> generatePuzzle(bool basic, int startingNum = 15){
    std::vector<int> puzzle(N*N, -1);
    if(basic){
        bool useEasy = true;
        if(useEasy) {
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
            //  2: 2,4,5
            //  3: 2,5,
            //  4: 4,6,
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

        } else {
            //A more complicated puzzle - this one took my computer about 15 minutes to solve
            //  =====================
            // ||*|*|*||*|*|8||*|*|*||
            // ||*|*|7||*|*|*||*|*|5||
            // ||*|*|*||3|*|*||*|*|7||
            //  =====================
            // ||*|*|*||*|1|*||6|*|*||
            // ||*|*|3||7|*|*||*|*|*||
            // ||*|2|*||*|*|*||*|8|*||
            //  =====================
            // ||*|*|*||5|*|*||*|4|3||
            // ||8|6|*||*|*|*||*|*|9||
            // ||*|*|*||*|*|*||*|*|*||
            //  =====================
            puzzle[5] = 8;
            puzzle[11] = 7;
            puzzle[17] = 5;
            puzzle[21] = 3;
            puzzle[26] = 7;
            puzzle[31] = 1;
            puzzle[33] = 6;
            puzzle[38] = 3;
            puzzle[39] = 7;
            puzzle[46] = 2;
            puzzle[52] = 8;
            puzzle[57] = 5;
            puzzle[61] = 4;
            puzzle[62] = 3;
            puzzle[63] = 8;
            puzzle[64] = 6;
            puzzle[71] = 9;
        }
    }else
    {
        int diagonalSeed = sqrt(N)+1;
        for(int i=0;i<N;++i){
            if(i%diagonalSeed==0) fillBox(puzzle, i);
        }

        solvePuzzle(puzzle);
        int removeAmount = 81-startingNum;
        while(removeAmount>0){
            int removeNum= rand() % 80;
            if(puzzle[removeNum]>0){
                puzzle[removeNum] = -1;
                removeAmount--;
            }
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
        for(int j = (i + N) % (N*N); j != i; j=(j+N) % ( N*N )){
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
    //std::cout << i << " was " << j << std::endl;
    return true;
}

int getWidth(int n){
    if (n < 10)
        return 1;
    if(n < 100)
        return 2;
    if(n < 1000)
        return 3;
    return 0;
}
void printPuzzle(std::vector<int> puzzle){
    int w = getWidth(N);
    int rowLength = N*2+box+(N-9);

    bool overNine = (w-1);
    
    std::cout<<" "<<std::string(rowLength, '=')<<std::endl;
    for(int i=0;i<puzzle.size();){
        for(int j=0;j<box;++j){
            for(int k=0;k<box;++k){
                std::cout<<"|";
                for(int l=0;l<box;++l){
                    if(puzzle[i]==-1) std::cout<<"|" << std::setw(w) << "*";
                    else std::cout<< "|" << std::setw(w) << puzzle[i];
                    ++i;
                }
            }
            std::cout<<"||"<<std::endl;
        }
        std::cout<<" "<<std::string(rowLength, '=')<<std::endl;
    }
}

//I'll give this a return value when it's closer to finished
void solvePuzzle(std::vector<int>& puzzle){
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

bool whyValid(std::vector<int> puzzle){
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
        for(int j = (i + N) % (N*N); j != i; j=(j+N) % ( N*N )){
            if(puzzle[j] == -1)
                continue;
            if(puzzle[i] == puzzle[j]){
                    std::cout << i << " was " << j << std::endl;
                    std::cout << puzzle[i] << " : " << puzzle[j] << std::endl;
                    printPuzzle(puzzle);
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
void generateQueue(std::vector<std::vector<int>> &queue, std::vector<int> puzzle){
    queue.push_back(puzzle);
    std::vector<int> currPuzzle;
    int start = 0;
    int loops = 0;
    int currIndex = start;
    while(puzzle[currIndex] != -1)
        ++currIndex;
    while(loops < 3 || (queue.size() < 100 && currIndex < N*N)){
        start = currIndex;
        int added = queue.size();
        for (int k = 0; k < added; ++k){
            //std::cout << " --- " << std::endl;
            currPuzzle = queue[0];
            queue.erase(queue.begin());
            //std::cout << "Popped " << std::endl;
            //printPuzzle(currPuzzle);

            if(k == 0){
            }
            for(int i = 1; i <= N; ++i){
                currPuzzle[currIndex] = i;
                if(isValid(currPuzzle)){
                    queue.push_back(currPuzzle);
                    std::cout << "Added " << i << " at " << currIndex << std::endl;
                    //if(i == 6) printPuzzle(currPuzzle);
                }
            }
        }
        while(queue.back()[currIndex] != -1){
            ++currIndex;
        }
        ++loops;
    }
    std::cout << "Done! " << queue.size() << std::endl;
    /*
    for(int i = 0; i < queue.size(); ++i){
        printPuzzle(queue[i]);
        std::cout << std::endl;
    }
    */
}

int main(int argc, char **argv){
    int rank, size, data;
    std::vector<int> puzzle, puzzle2;
    

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank); 
    MPI_Comm_size(MCW, &size); 
    srand(rank+time(0));

    //MPI_Send(&rank, 1, MPI_INT, (rank+1)%size, 0, MCW);
    //MPI_Recv(&data, 1, MPI_INT, MPI_ANY_SOURCE, 0, MCW, MPI_STATUS_IGNORE);

    int TAG_PUZZLE = 0;
    int TAG_QUANTITY = 1;
    int TAG_MORE = 2;
    int TAG_SOLVED = 3;
    int TAG_POISON = 4;


    if(rank==0){
        std::vector<std::vector<int>> queue;
        puzzle = generatePuzzle(false, 17);
        generateQueue(queue, puzzle);
        //solvePuzzle(puzzle);
        int currIndex = 0;
        int quantity = 0;
        int remaining = queue.size();
        for(int i = 1; i < size && remaining > 0; ++i){
            while (quantity < 4 && remaining > 0){
                ++quantity
                --remaining;
            }
            MPI_Send(&quantity, 1, MPI_INT, i, TAG_QUANTITY, MCW);
            for (int j = 0; j < quantity; ++j){
                MPI_Send(queue[currIndex+j].data(), queue[j].size(), MPI_INT, i, TAG_PUZZLE, MCW);
            }
            currIndex += quantity;
        }
        


        //std::cout << queue.size() << std::endl;

        /*
        puzzle = generatePuzzle(true);
        printPuzzle(puzzle);

        if(!isValid(puzzle)){
            printf("Invalid puzzle\n");
        } else printf("Seems valid\n");
        //solvePuzzle(puzzle);

        MPI_Send(puzzle.data(), puzzle.size(), MPI_INT, 1, 0, MCW);
        std::cout << "Communication complete" << std::endl;
        /*
        puzzle2 = generatePuzzle(false, 17);
        std::cout<<"Randomly generated puzzle:"<<std::endl;
        printPuzzle(puzzle2);
        std::cout<<"Puzzle after being solved:"<< std::endl;
        solvePuzzle(puzzle2);
        */
    } else {
        std::vector<std::vector<int>> queue;

        MPI_Status status;
        bool isIncoming = false;
        int inc;
        bool isDone = false;
        while(!isDone){
            MPI_Iprobe(0, MPI_ANY_TAG, MCW, &isIncoming, &status);
            if(isIncoming){
                switch(status.MPI_TAG){
                    case TAG_POISON:    //Recieve poison pill
                        MPI_Recv(&inc, 1, MPI_INT, 0, TAG_POISON, MCW, MPI_STATUS_IGNORE);
                        isDone = true;
                        break;
                    case TAG_QUANTITY:  //Recieve new puzzles
                        int quantity;
                        std::vector<int> data (N*N);
                        MPI_Recv(&quantity, 1, MPI_INT, 0, TAG_QUANTITY, MCW, MPI_STATUS_IGNORE);
                        for(int i = 0; i < quantity; ++i){
                            MPI_Recv(data.data(), data.size(), MPI_INT, 0, TAG_PUZZLE, MCW, MPI_STATUS_IGNORE);
                            queue.push_back(data);
                        }
                        break;
                    default:
                        break;
                }
            }
            if(!isDone){
                //Attempt to solve a puzzle, or request more
            }
        }
        
        /*
        MPI_Recv(puzzle.data(), puzzle.size(), MPI_INT, MPI_ANY_SOURCE, 0, MCW, MPI_STATUS_IGNORE);
        std::cout << std::endl << std::endl;
        std::cout << "Rank 1: " << std::endl;
        printPuzzle(puzzle);
        std::cout << "Communication complete" << std::endl;
        */
    }

exit:
    MPI_Finalize();

    return 0;
}
