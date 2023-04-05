#include <stdio.h>
#include <stdbool.h>
#include <mpi.h>
#include <string.h>
#include <math.h>

#define max 100

const int N = 100;
int n; // Number of cities
int initial_city; // Starting city for calculating best cost
int dist[max][max]; // Distance matrix
int curr_path[max]; // Current path
int best_path[max]; // Best path
int curr_bound; // Current bound
int bound;
bool visited[max]; // To mark visited cities
int ctr1 = 0;

/*
Function: WSP
--------------
Function to solve wandering salesman problem using branch and bound algorithm

vertex: Current position/vertex of the salesman 
length: No. of cities visited
cost: Current cost of the path
temp_cost: Stores the cost of the current path
best_cost: Best cost of the path
bound: Current value of the bound
visited: maxtrix which has values of visited cities

returns: None
*/

void WSP(int vertex, int length, int task_id) 
{
    if (length == n) // Checks if all cities are visited
    {
        if (curr_bound <= bound) // Update the bound if the current bound is minimum
        {
            bound = curr_bound;
            for (int i = 0; i < n; i++)
            {
                best_path[i] = curr_path[i];
            }
        }
        return;
    }
    
    for (int v = 0; v < n; v++) // Checks every unvisited city
    {
        if (length == 2 && v == 0)
        {
            ctr1 = vertex;
            while (ctr1 != 0)
            {
                if (ctr1 - 1 != 0)
                {
                    visited[ctr1 - 1] = false;
                }
                ctr1--;
            }
        }
        if (dist[vertex][v] != -1 && !visited[v])
        {
            int temp_bound = curr_bound + dist[vertex][v]; // The cost of travelling next city is added
            if (temp_bound < bound) // Check if the cost of current path is minimum
            {
                curr_path[length] = v;
                visited[v] = true;
                curr_bound += dist[vertex][v];
                WSP(v, length + 1, task_id); // Solve the WSP using branch and bound algorithm
                curr_bound -= dist[vertex][v];
                visited[v] = false;
                if (length == 1)
                {
                    return;
                }
            }
        }
    }
}

int main(int argc, char* argv[]) 
{
    int start, end;
    int temp_bound1 = 10000;
    int temp_bound2;
    int temp_path1[N];
    int temp_path2[N];
    int temp = 10000;
    int temp_p[N];
    int num_tasks;
    int task_id;
    double t0,t1,t2,t3,time_spent,comm_time = 0.0;
    char file_name[100];
    
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_tasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &task_id);
    
    if (task_id == 0)
    {
        printf("\n Enter a valid filename (dist4 to dist17): ", file_name);
        scanf("%s",file_name);
        FILE *file = fopen(file_name, "r"); // Opens the input file
        fscanf(file, "%d", &n); // Read the no. of cities from the file
        
        for (int i = 0; i < n; i++) // Read the lower triangular matrix from the file
        {
            for (int j = 0; j <= i; j++)
            {
                if (i == j)
                {
                }
                else
                {
                    fscanf(file, "%d", &dist[i][j]);
                }
            }
        }
        
        for (int i = 0; i < n; i++) // Creates upper triangular matrix
        {
            for (int j = i + 1; j < n; j++)
            {
                if (i == j)
                {
                    dist[i][j] = 0; // Diagonal elements in the distance matrix is set to 0
                }
                else
                {
                    dist[i][j] = dist[j][i];
                }
            }
        }
        
        fclose(file); // Closes the input file
        
        printf("\n The distance matrix is:\n");
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                printf("%d ", dist[i][j]); // Prints the distance matrix
            }
        printf("\n");
        }
        
        memset(curr_path, -1, sizeof curr_path);
        memset(best_path, -1, sizeof best_path);
        memset(visited, false, sizeof visited);
        
        curr_bound = 0;
        bound = 1000000;
        printf("\n Enter the initial city (0 to %d): ",n-1);
        scanf("%d", &initial_city);
        
        if(initial_city < 0 || initial_city >= n)
        {
            printf("City doesn't exist. Kindly enter a valid city (0 to %d): ",n-1);
            scanf("%d", &initial_city);
        }
        curr_path[0] = initial_city;
        visited[initial_city] = true;
        WSP(initial_city, 1, task_id);
    }
    
    memset(curr_path, -1, sizeof curr_path);
    memset(best_path, -1, sizeof best_path);
    memset(visited, false, sizeof visited);
    
    curr_bound = 0;
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&dist, N * N, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&bound, 1, MPI_INT, 0, MPI_COMM_WORLD);

    t0 = MPI_Wtime(); // Begin computation time
    
    int remaining_cities = n % num_tasks;
    int block_size;
    if (n >= num_tasks)
    {
        block_size = ceil(n / num_tasks);
        start = task_id * block_size;
        end = start + block_size - 1;
    }
    else
    {
        block_size = 1;
        start = task_id;
        end = start;
    }
    if (task_id < remaining_cities)
    {
        start += task_id;
        end = start + block_size;
    }
    else
    {
        start += remaining_cities;
        end = start + block_size - 1;
    }
    for (int i = start; i <= end; i++)
    {
        for (int k = 0; k <= i; k++)
        {
            visited[k] = true;
        }
        curr_path[initial_city] = 0;
        WSP(initial_city, 1, task_id);
        if (bound < temp_bound1)
        {
            temp_bound1 = bound;
            for (int i = 0; i < n; i++)
            {
                temp_path1[i] = best_path[i];
            }
        }
    }
    
    t1 = MPI_Wtime(); // End computation time
    time_spent = t1-t0; // Calculate computation time

    t2 = MPI_Wtime(); // Begin communication time
    MPI_Send(&temp_bound1, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
    MPI_Send(temp_path1, 100, MPI_INT, 0, 3, MPI_COMM_WORLD);
    
    if (task_id == 0)
    {
        int temp = 10000;
        for (int i = 1; i < num_tasks; i++)
        {
            MPI_Recv(&temp_bound2, 1, MPI_INT, i, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(temp_path2, 100, MPI_INT, i, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (temp_bound2 < temp)
            {
                temp = temp_bound2;
                for (int j = 0; j < n; j++)
                {
                    temp_path1[j] = temp_path2[j];
                }
            }
        }
        
        t3 = MPI_Wtime(); // End communication time
        comm_time = t3-t2; // Calculate communication time
        
        printf("\n Best cost for the given path: %d \n", temp); // Print the best cost
        printf("\n Best path for the given no. of cities: "); // Print the best path found
        for (int i = 0; i < n; i++)
        {
            printf("%d ", temp_path1[i]);
        }
        printf("\n Execution Time: %.4f seconds \n", time_spent); // Print the Execution time
        printf("\n Communication Time: %.4f seconds \n", comm_time); // Print the Communication time 
    }
    MPI_Finalize();
    return 0;
}
