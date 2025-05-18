/**
 * @file po2csv.h
 * @brief Header file for PO to CSV converter utility
 *
 * Copyright 2025 Your Name
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

 #ifndef PO2CSV_H
 #define PO2CSV_H
 
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 
 /**
  * @brief Print usage information for the program
  *
  * @param program_name Name of the executable
  */
 void print_usage(const char *program_name);
 
 /**
  * @brief Convert a PO file to CSV format
  *
  * @param input_file Path to the input PO file
  * @param output_file Path to the output CSV file
  * @return int 0 on success, non-zero on failure
  */
 int po2csv_convert(const char *input_file, const char *output_file);
 
 /**
  * @brief Escape special characters in CSV
  * 
  * @param input Input string
  * @param output Output buffer
  * @param output_size Size of output buffer
  * @return int Length of the escaped string
  */
 int po2csv_escape_csv(const char *input, char *output, size_t output_size);
 
 /**
  * @brief Parse a quoted string from a PO file line
  * 
  * @param line Line from PO file
  * @return char* Pointer to allocated string (must be freed by caller), NULL on error
  */
 char *parse_quoted_string(char *line);
 
 /**
  * @brief Process a continuation line (starting with quote)
  * 
  * @param line Line from PO file
  * @param buffer Buffer to append content to
  * @param buffer_size Size of the buffer
  * @return int 1 on success, 0 on failure
  */
 int process_continuation(char *line, char *buffer, size_t buffer_size);
 
 #endif /* PO2CSV_H */