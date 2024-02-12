//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "utils.h"

std::vector<std::vector<double>> Utils::csv_to_matrix(std::string input_csv)
{
    // Open the input file
    std::ifstream inputFile(input_csv);
    std::vector<std::vector<double>> matrix;

    // Read the file line by line
    std::string line;
    while (getline(inputFile, line)) {
        // Create a stringstream from the line
        std::stringstream lineStream(line);

        // Create a vector to hold the current row of the matrix
        std::vector<double> row;

        // Read the values from the line, separated by tabs
        double value;
        while (lineStream >> value) {
            row.push_back(value);

            if (lineStream.peek() == '\t') {
                lineStream.ignore();
            }
        }

        // Add the row to the matrix
        matrix.push_back(row);
    }

    // Close the input file
    inputFile.close();

    return matrix;
}
