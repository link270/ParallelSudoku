#include <algorithm>
#include <iostream>
#include <mpi.h>
#include <math.h>
#include <vector>
#include <random>
#include <iomanip>
#include <chrono>

#define MCW MPI_COMM_WORLD

#define N 9
#define box 3

void fillBox(std::vector<int> &puzzle);
std::vector<int> generatePuzzle(bool basic);
void generateQueue(std::vector<std::vector<int> > &queue, std::vector<int> puzzle);
int getBox(int puzzleIndex);
int getColumn(int puzzleIndex);
int getRow(int puzzleIndex);
bool isValid(std::vector<int> puzzle);
void printPuzzle(std::vector<int> puzzle);
void report(std::vector<long long> allCompletionTimes);
bool solvePuzzle(std::vector<int> &puzzle);

const int TAG_PUZZLE = 0;
const int TAG_QUANTITY = 1;
const int TAG_MORE = 2;
const int TAG_SOLVED = 3;
const int TAG_POISON = 4;
const int TAG_ACK = 5;

void fillBox(std::vector<int> &puzzle, int boxNum)
{
    std::vector<int> values;
    for (int i = 0; i < N; ++i)
    {
        values.push_back(i);
    }

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(values.begin(), values.end(), g);
    int i = 0;
    while (!values.empty())
    {
        if (getBox(i) == boxNum)
        {
            puzzle[i] = values.back() + 1;
            values.pop_back();
        }
        ++i;
    }
}

// StartingNum must be 17 or higher to get a unique solution
std::vector<int> generatePuzzle(bool basic)
{
    std::vector<int> puzzle(N * N, -1);
    if (basic)
    {
        bool useEasy = false;
        if (useEasy)
        {
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
        }
        else
        {
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
    }
    else
    {
        int diagonalSeed = sqrt(N) + 1;
        for (int i = 0; i < N; ++i)
        {
            if (i % diagonalSeed == 0)
                fillBox(puzzle, i);
        }

        solvePuzzle(puzzle);
        std::random_device rd;
        std::mt19937 eng(rd());
        std::uniform_int_distribution<> distr(0, (N*N)-1);
        int startingNum = 17;
        if(N==16) startingNum = 60;
        int removeAmount = N*N - startingNum;
        while (removeAmount > 0)
        {
            int removeNum = distr(eng);
            if (puzzle[removeNum] > 0)
            {
                puzzle[removeNum] = -1;
                removeAmount--;
            }
        }
    }

    return puzzle;
}

void generateQueue(std::vector<std::vector<int> > &queue, std::vector<int> puzzle)
{
    queue.push_back(puzzle);
    std::vector<int> currPuzzle;
    int start = 0;
    int loops = 0;
    int currIndex = start;
    while (puzzle[currIndex] != -1)
        ++currIndex;
    while (loops < 3 || (queue.size() < 100 && currIndex < N * N))
    {
        start = currIndex;
        int added = queue.size();
        for (int k = 0; k < added; ++k)
        {
            currPuzzle = queue[0];
            queue.erase(queue.begin());

            if (k == 0)
            {
            }
            for (int i = 1; i <= N; ++i)
            {
                currPuzzle[currIndex] = i;
                if (isValid(currPuzzle))
                {
                    queue.push_back(currPuzzle);
                }
            }
        }
        while (queue.back()[currIndex] != -1)
        {
            ++currIndex;
        }
        ++loops;
    }
}

// Gives the box number of the puzzle index.
// Box numbers increase from
int getBox(int puzzleIndex)
{
    return (puzzleIndex % N) / box + box * (puzzleIndex / (N * box));
}

// Give you the column of the puzzle index
// Columns start at 0-8
int getColumn(int puzzleIndex)
{
    return puzzleIndex % N;
}

// Give you the row of the puzzle index
// Rows start at 0-8
int getRow(int puzzleIndex)
{
    return puzzleIndex / N;
}

bool isIndexValid(std::vector<int> &puzzle, int i)
{
    //Row is all indices in [Nx(i/3), +N]
    //Values in row are found by adding 1, modulus N, then adding to get back to the correct row if necessary
    int row = getRow(i);
    for (int j = (i + 1) % N + (N * row); j != i; j = ((++j) % N) + (N * row))
    {
        if (puzzle[j] == -1)
            continue;
        if (puzzle[i] == puzzle[j])
        {
            return false;
        }
    }

    //Values in column are found by adding/subtracting N
    for (int j = (i + N) % (N * N); j != i; j = (j + N) % (N * N))
    {
        if (puzzle[j] == -1)
            continue;
        if (puzzle[i] == puzzle[j])
        {
            return false;
        }
    }

    //Values in box are found by finding the minimum and maxiumum possible index that could share a box, and iterating from there.
    int currBox = getBox(i);
    int start = i - (N * box) - box;
    start = (start < 0) ? 0 : start;
    int end = i + (N * box) + box;
    end = (end < N * N) ? end : N * N;
    for (int j = start; j < end; ++j)
    {
        int jBox = getBox(j);
        if (j != i && puzzle[j] != -1 && jBox == currBox)
        {
            if (puzzle[i] == puzzle[j])
            {
                return false;
            }
        }
    }
    return true;
}

bool isValid(std::vector<int> puzzle)
{
    //iterate over all values
    for (int i = 0; i < N * N; ++i)
    {
        //cell not filled, move onto the next one
        if (puzzle[i] == -1)
        {
            continue;
        }
        if (!isIndexValid(puzzle, i))
        {
            return false;
        }
    }
    //std::cout << i << " was " << j << std::endl;
    return true;
}

int getWidth(int n)
{
    if (n < 10)
        return 1;
    if (n < 100)
        return 2;
    if (n < 1000)
        return 3;
    return 0;
}
void printPuzzle(std::vector<int> puzzle)
{
    int w = getWidth(N);
    int rowLength = N * 2 + box + (N - 9);

    bool overNine = (w - 1);

    std::cout << " " << std::string(rowLength, '=') << std::endl;
    for (int i = 0; i < puzzle.size();)
    {
        for (int j = 0; j < box; ++j)
        {
            for (int k = 0; k < box; ++k)
            {
                std::cout << "|";
                for (int l = 0; l < box; ++l)
                {
                    if (puzzle[i] == -1)
                        std::cout << "|" << std::setw(w) << "*";
                    else
                        std::cout << "|" << std::setw(w) << puzzle[i];
                    ++i;
                }
            }
            std::cout << "||" << std::endl;
        }
        std::cout << " " << std::string(rowLength, '=') << std::endl;
    }
}

void report(std::vector<long long> allCompletionTimes){
    int totalTime = 0;
    for (int i = 0; i < allCompletionTimes.size(); ++i)
    {
        totalTime += allCompletionTimes[i];
        std::cout << "\nPuzzle number: " << i + 1 << " was solved in " << allCompletionTimes[i] << " microseconds.\n";
    }

    long long averageTime = totalTime / allCompletionTimes.size();
    std::sort(allCompletionTimes.begin(), allCompletionTimes.end());
    std::cout << "\nAll " << allCompletionTimes.size() << " puzzles were solved in a total time of " << totalTime << " microseconds.\n";
    std::cout << "The longest puzzle took " << allCompletionTimes.back() << " microseconds to solve and the shortest taking ";
    std::cout << allCompletionTimes[0] << " microseconds\n";
    std::cout << "With an average time of " << averageTime << " microseconds accross all puzzles.\n";
}

//I'll give this a return value when it's closer to finished
bool solvePuzzle(std::vector<int> &puzzle)
{
    std::vector<int> queue;
    int isIncoming = false;
    int start = 0;
    while (puzzle[start] != -1)
    {
        ++start;
    }
    queue.push_back(start);
    int curr = queue.back();
    while (queue.back() < N * N)
    {
        int i = 1;
        while (1)
        {
            if (queue.size() == 0)
                break;
            puzzle[queue.back()] = i;
            if (!isIndexValid(puzzle, queue.back()))
            {
                ++i;
                while (i > N)
                {
                    puzzle[queue.back()] = -1;

                    queue.pop_back();
                    if (queue.size() > 0)
                        i = puzzle[queue.back()] + 1;
                    else
                        break;
                }
                MPI_Iprobe(0, TAG_POISON, MCW, &isIncoming, MPI_STATUS_IGNORE);
                if(isIncoming){
                    return false;
                }
            }
            else
            {
                break;
            }
        }

        if (queue.size() > 0)
            curr = queue.back();
        else
            return false;
        do
        {
            curr++;
        } while (curr < N * N && puzzle[curr] != -1);
        queue.push_back(curr);
    }
    return true;
}

int main(int argc, char **argv)
{
    int rank, size;
    std::vector<int> puzzle, puzzle2;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);
    srand(rank + time(0));


    int workerQueueSize = 8;
    long long completionTime;

    int timesToRun = 10;
    std::vector<long long> allCompletionTimes;

    if(argc > 1) timesToRun = strtol(argv[1], nullptr, 0);

    while (timesToRun > 0)
    {
        MPI_Barrier(MCW);
        if (rank == 0)
        {
            std::cout <<"Starting" << std::endl;
            bool isDone = false;

            //Generate queue
            std::vector<std::vector<int> > queue;
            puzzle = generatePuzzle(true);
            std::cout << "Puzzle to be solved: " << std::endl;
            printPuzzle(puzzle);
            std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
            generateQueue(queue, puzzle);

            //Send initial puzzles
            int currIndex = 0;
            int quantity = 0;
            int remaining = queue.size();
            for (int i = 1; i < size && remaining > 0; ++i)
            {
                while (quantity < workerQueueSize && remaining > 0)
                {
                    ++quantity;
                    --remaining;
                }
                MPI_Send(&quantity, 1, MPI_INT, i, TAG_QUANTITY, MCW);
                for (int j = 0; j < quantity; ++j)
                {
                    MPI_Send(queue[currIndex + j].data(), N*N, MPI_INT, i, TAG_PUZZLE, MCW);
                }
                currIndex += quantity;
                quantity = 0;
            }

            MPI_Status status;
            std::vector<int> data(N * N);
            int inc = -1;
            int isIncoming = 0;
            //While not done
            while (!isDone)
            {

                //Check for incoming messages
                MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MCW, &status);

                //If a worker has found the solution
                if (status.MPI_TAG == TAG_SOLVED)
                {
                    MPI_Recv(data.data(), N*N, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MCW, MPI_STATUS_IGNORE);
                    isDone = true;
                    std::cout << "Worker " << status.MPI_SOURCE << " solved the puzzle: " << std::endl;
                    std::chrono::steady_clock::time_point endTime = std::chrono::steady_clock::now();
                    completionTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
                    printPuzzle(data);

                    //Send out poison pills
                    for (int i = 1; i < size; ++i)
                    {
                        //std::cout << "Waiting on " << i << std::endl;
                        MPI_Request ack;
                        MPI_Request poison;
                        int wasAcked = false;
                        MPI_Irecv(&inc, 1, MPI_INT, i, TAG_ACK, MCW, &ack);
                        std::vector<MPI_Request> pills;
                        do
                        {
                        MPI_Isend(&inc, 1, MPI_INT, i, TAG_POISON, MCW, &poison);
                            //Start nonblocking send to i
                            MPI_Status blocker;
                            //Check to make sure i isn't sending something already,
                            //or to see if the send has finished
                            int isPoisonComplete = 0;
                            isIncoming = 0;
                            while (!isIncoming && !isPoisonComplete)
                            {
                                MPI_Test(&poison, &isPoisonComplete, MPI_STATUS_IGNORE);
                                MPI_Iprobe(i, TAG_MORE, MCW, &isIncoming, &blocker);
                                if(!isIncoming){
                                    MPI_Iprobe(i, TAG_SOLVED, MCW, &isIncoming, &blocker);
                                }
                            }
                            //If i is trying to send, recieve it
                            if (isIncoming)
                            {
                                //Handle incoming message
                                if (blocker.MPI_TAG == TAG_MORE)
                                {
                                    MPI_Recv(&inc, 1, MPI_INT, blocker.MPI_SOURCE, blocker.MPI_TAG, MCW, MPI_STATUS_IGNORE);
                                }
                                else if (status.MPI_TAG == TAG_SOLVED)
                                {
                                    MPI_Recv(data.data(), N*N, MPI_INT, blocker.MPI_SOURCE, status.MPI_TAG, MCW, MPI_STATUS_IGNORE);
                                }
                            }
                            //Wait for i to recieve the poison pill so we can continue
                            MPI_Test(&ack, &wasAcked, MPI_STATUS_IGNORE);
                        } while (!wasAcked);
                    }
                }
                //If nobody has found a solution, if someone wants more work:
                else if (status.MPI_TAG == TAG_MORE)
                {
                    MPI_Recv(&inc, 1, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MCW, MPI_STATUS_IGNORE);
                    while (quantity < workerQueueSize && remaining > 0)
                    {
                        ++quantity;
                        --remaining;
                    }
                    MPI_Send(&quantity, 1, MPI_INT, status.MPI_SOURCE, TAG_QUANTITY, MCW);
                    for (int j = 0; j < quantity; ++j)
                    {
                        MPI_Send(queue[currIndex + j].data(), queue[j].size(), MPI_INT, status.MPI_SOURCE, TAG_PUZZLE, MCW);
                    }
                    currIndex += quantity;
                }
                //End loop
            }
        }
        else
        {
            std::vector<std::vector<int> > queue;
            int workingIndex = 0;
            bool isDone = false;
            int isIncoming = 0;
            int inc = 0;
            MPI_Status status;
            //While not done
            while (!isDone)
            {
                //Recieve new work if necessary
                if (workingIndex == queue.size())
                {
                    MPI_Request pill, puzzles;
                    workingIndex = 0;
                    queue.clear();
                    int quantity;
                    std::vector<int> data(N * N);
                    int puzzlesFlag = 0, pillFlag = 0;
                    MPI_Irecv(&quantity, 1, MPI_INT, 0, TAG_QUANTITY, MCW, &puzzles);
                    MPI_Irecv(&inc, 1, MPI_INT, 0, TAG_POISON, MCW, &pill);
                    while (!puzzlesFlag && !pillFlag)
                    {
                        MPI_Test(&pill, &pillFlag, MPI_STATUS_IGNORE);
                        MPI_Test(&puzzles, &puzzlesFlag, MPI_STATUS_IGNORE);
                    }
                    if (puzzlesFlag)
                    {
                        for (int i = 0; i < quantity; ++i)
                        {
                            MPI_Recv(data.data(), N*N, MPI_INT, 0, TAG_PUZZLE, MCW, MPI_STATUS_IGNORE);
                            queue.push_back(data);
                        }
                        MPI_Cancel(&pill);
                    }
                    else
                    {
                        MPI_Cancel(&puzzles);
                    }
                }

                //Do work
                if (queue.size() > 0)
                {
                    isDone = solvePuzzle(queue[workingIndex]);
                    //Report done if necessary
                    if (isDone)
                    {
                        MPI_Send(queue[workingIndex].data(), queue[workingIndex].size(), MPI_INT, 0, TAG_SOLVED, MCW);
                    }
                    //Request more work if necessary
                    else if (workingIndex + 1 == queue.size())
                    {
                        MPI_Send(&inc, 1, MPI_INT, 0, TAG_MORE, MCW);
                    }
                    ++workingIndex;
                }

                //Check for poison pill
                MPI_Iprobe(0, TAG_POISON, MCW, &isIncoming, &status);
                //Recieve poison pill if necessary
                if (isIncoming || isDone)
                {
                    MPI_Recv(&inc, 1, MPI_INT, 0, TAG_POISON, MCW, MPI_STATUS_IGNORE);
                    isDone = true;
                }
                //End loop
            }
            MPI_Send(&inc, 1, MPI_INT, 0, TAG_ACK, MCW);
        }
        MPI_Barrier(MCW);
        if (rank == 0)
        {
            std::cout << "Time from puzzle creation to puzzle solution was " << completionTime << " microseconds.\n";
            allCompletionTimes.push_back(completionTime);
        }

        timesToRun--;

        int t = 0, d = 0;
        MPI_Status cleanup;
        MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MCW, &t, &cleanup);
        while (t)
        {
            MPI_Recv(&d, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MCW, &cleanup);
            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MCW, &t, &cleanup);
        }
    }

exit:

    MPI_Barrier(MCW);
    if (rank == 0)
    {
        report(allCompletionTimes);
    }

    MPI_Finalize();

    return 0;
}
