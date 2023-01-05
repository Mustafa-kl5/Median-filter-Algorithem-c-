#include <iostream>
#include<fstream>
#include <pthread.h>
#include <vector>
#include <algorithm>
#include <mutex>

using namespace std;

int dimensions=0;
int numOfBright=0;
int numOfDark=0;
int numOfNormal=0;
pthread_mutex_t m;

struct MedianFilterData
{
    const vector<vector<int>>* inputImage;
    vector<vector<int>>* outputImage;
    int startRow;
    int endRow;
    int threadId;
};

void* medianFilter(void* data)
{    
 // Cast the void pointer to the correct type
    MedianFilterData* filterData = (MedianFilterData*) data;

    // Access the data from the structure
    const vector<vector<int>>& inputImage = *filterData->inputImage;
    vector<vector<int>>& outputImage = *filterData->outputImage;
    int startRow = filterData->startRow;
    int endRow = filterData->endRow;
    int threadId = filterData->threadId;
  
    for (int i = startRow; i <= endRow; i++)
{
    // Loop through the columns in this row
    for (int j = 0; j < inputImage[i].size(); j++)
    {
        // Vector to store the values of the 9 pixels
        // that we will use to calculate the median
        vector<int> values;

        // Loop through the 3x3 window around the current pixel
        for (int k = i - 1; k <= i + 1; k++)
        {
            for (int l = j - 1; l <= j + 1; l++)
            {
                // Check if the current position is within the bounds of the image
                if (k >= 0 && k < inputImage.size() && l >= 0 && l < inputImage[k].size())
                {
                    // Add the pixel value to the vector
                    values.push_back(inputImage[k][l]);
                }
                else
                {
                    // Add a zero value for missing neighbors
                    values.push_back(0);
                }
            }
        }

        // Sort the vector to find the median value
        sort(values.begin(), values.end());

        // Set the median value as the value of the current pixel in the output image
        outputImage[i][j] = values[values.size() / 2];

        // Update the statistics based on the value of the current pixel
        if (outputImage[i][j] > 200)
        {
            // Lock the mutex before updating the global variable
           numOfBright++;
        }
        else if (outputImage[i][j] < 50)
        {
            // Lock the mutex before updating the global variable
            numOfDark++;
        }
        else
        {
            // Lock the mutex before updating the global variable
            numOfNormal++;
        }
    }
}

  // Print statistics for each thread
        cout<<"ThreadID="<<threadId<<",startRow="<<startRow<<",endRow="<<endRow+1<<endl;
        cout << "number of bright pixels= " << numOfBright << ","<< "Number of dark    pixels="<<numOfDark << ","<< "Number of normal pixels= " << numOfNormal << endl;
        
     return nullptr;   
}




vector<vector<int>> readData(const string& fileName)
{      
          // Open the file
  ifstream file(fileName);

  // Check if the file was successfully opened
  if (!file.is_open()) {
    cerr << "Error opening file: " << fileName << endl;
    
  }


  // Create a 2D vector to store the matrix
  vector<vector<int>> matrix(dimensions, vector<int>(dimensions));

  // Read the matrix elements
  for (int i = 0; i < dimensions; i++) {
    for (int j = 0; j < dimensions; j++) {
      file >> matrix[i][j];
    }
  }

  // Close the file
  file.close();

  // Return the matrix
  return matrix;
}
void writeData(const string& fileName, const vector<vector<int>>& data)
{
    // Open the output file
    ofstream out(fileName);

    // Check if the file was opened successfully
    if (!out.is_open())
    {
        cerr << "Error: Could not open file " << fileName << endl;
        return;
    }

    // Write the data to the file
    for (int i = 0; i < data.size(); i++)
    {
        for (int j = 0; j < data[i].size(); j++)
        {   
             string temp=to_string(data[i][j]);
        if(temp.size()==3){
            out << data[i][j]<<" ";
        
        }
        else if(temp.size()==2){
         out << data[i][j]<<"   ";
      
        
        
        }else{
         out << data[i][j]<<"   ";
        }
        }

        // Add a newline character at the end of each row
        out << "\n";
    }

    // Close the output file
    out.close();
}

int main(int argc, char** argv)  
{if (argc < 2)
    {
        cout << "Error: Please provide the number of worker threads" << endl;
        return 1;
    }

    // Read the number of worker threads from the command line
    int numOfT = atoi(argv[1]);
 
   
   // Read the input dimensions matrix 
    cout<<"Enter dimensions"<<endl;
    cin>>dimensions;
    
   // Read the input image
    vector<vector<int>> inputImage = readData("in.txt");
   
   // Create the output image
    vector<vector<int>> outputImage(dimensions, vector<int>(dimensions));
   
  
 // Vector to store the threads
    vector<pthread_t> threads;

    // Divide the image into equal-sized regions for each thread to process
    int rowsPerThread = inputImage.size() /  numOfT;
    
    // Create an array to store the data for each thread
       MedianFilterData data[numOfT];
       
       

    // Create and start the threads
    for (int i = 0; i < numOfT; i++)
    
    {       //fill MedianFilterData array data
           data[i].inputImage = &inputImage;
           data[i].outputImage = &outputImage;
           data[i].startRow = i * rowsPerThread;
           data[i].endRow = (i + 1) * rowsPerThread - 1;
           data[i].threadId = i;
    
        // Create a new thread
            pthread_t thread;
            pthread_create(&thread, NULL, medianFilter, (void*) &data[i]);

       // Add the thread to the vector
           threads.push_back(thread);
        
    }

    // Wait for all threads to finish
    for (int i = 0; i < numOfT; i++)
{
    // Wait for the thread to finish
    pthread_join(threads[i], NULL);
}

    // Print the main statistics
    cout<<"Main:";
    cout << "number of bright pixels= " << numOfBright << ",";
    cout << "Number of dark pixels= " << numOfDark << ",";
    cout << "Number of normal pixels= " << numOfNormal << endl;

    // Write the output image to a file
    writeData("out.txt",outputImage);

    return 0;
}
    

