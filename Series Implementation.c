#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>

#define max 100

int n; // Number of cities
int dist[max][max]; // Distance matrix
int path[max]; // Current path
int visited[max]; // To mark visited cities
int bound = 0; // Lower bound of current path
int best_cost = INT_MAX; // Best cost
int best_path[max]; // Best path

/*
Function: BOUND
----------------
Function to calculate the lower bound of the current path

vertex: Current position/vertex of the salesman 
bound: Current value of the bound
visited: maxtrix which has values of visited cities

returns: bound
*/

int BOUND(int vertex) 
{
    int bound = 0;
    visited[vertex] = 1; // Mark the current vertex as visited
    for (int i = 0; i < n - 1; i++) // Start from the current vertex and travels to the nearest unvisited vertex
    {
        int k = -1;
        for (int j = 0; j < n; j++) 
        {
            if (!visited[j] && (k == -1 || dist[vertex][j] < dist[vertex][k])) 
            {
                k = j;
            }
        }
        vertex = k;
        bound += dist[vertex][k];
    }
    return bound;
}

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

void WSP(int vertex, int length, int cost) 
{
    if (length == n) // Checks if all cities are visited
    {
        if (cost <= best_cost) // Update the best cost if the current cost is minimum
        {
            best_cost = cost;
            memcpy(best_path, path, sizeof(path));
        }
    } 
    
    else 
    {
        for (int i = 0; i < n; i++) // Checks every unvisited city
        {
            if (!visited[i]) 
            {
                int temp_cost = cost + dist[path[length - 1]][i]; // The cost of travelling next city is added
                if (temp_cost + bound < best_cost) // Check if the cost of current path is minimum
                {
                    path[length] = i;
                    visited[i] = 1;
                    WSP(i, length + 1, temp_cost);
                    visited[i] = 0;
                }
            }
        }
    }
}

int main(int argc, char* argv[]) 
{
    int initial_city = 0;
    char file_name[100];
    printf("\n Enter a valid filename (dist4 to dist17): ", file_name);
    scanf("%s",file_name);
    FILE *file = fopen(file_name, "r"); // Opens the input file
    fscanf(file, "%d", &n); // Read the no. of cities from the file

    for (int i = 0; i < n; i++) // Read the lower triangular matrix from the file
    {
        for (int j = 0; j <= i; j++) 
        {
            if(i==j)
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
        for (int j = i+1; j < n; j++) 
        {
            if(i==j)
            {
                dist[i][j] = 0; // Diagonal elements of the distance matrix is set to 0
            }
        
            else
            {
                dist[i][j] = dist[j][i];
            }
        }
    }

    printf("\n The distance matrix is:\n");
    for (int i = 0; i < n; i++) 
    {
        for (int j = 0; j < n; j++) 
        {
            printf("%d ", dist[i][j]); // Prints distance matrix
        }
        printf("\n");
    }

    fclose(file); // Closes the input file

    printf("\n Enter the initial city (0 to %d): ",n-1);
    scanf("%d", &initial_city);
    if(initial_city < 0 || initial_city >= n)
    {
        printf("City doesn't exist. Kindly enter a valid initial city(0 to %d) only: ",n-1);
        scanf("%d", &initial_city);
    }
    
    clock_t begin = clock(); // Begin computation time
    
    path[initial_city] = 0;
    visited[initial_city] = 1;
    bound = BOUND(initial_city); // Calculate the lower bound of the path
    WSP(initial_city, 1, 0); // Solve the WSP using branch and bound algorithm
    
    clock_t end = clock(); // End computation time
    
    printf("\n Best path for the given no. of cities: "); // Print the best path found
    for (int i = 0; i < n; i++) 
    {
        printf("%d ", best_path[i]);
    }
    printf("\n Best cost for the given path: %d \n", best_cost);
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC; // Calculates Execution time
    printf("\n Execution Time: %f seconds\n" , time_spent); // Prints Execution time

    return 0;
}
