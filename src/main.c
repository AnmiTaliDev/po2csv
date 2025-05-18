/**
 * @file main.c
 * @brief Main implementation for PO to CSV converter utility
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

 #include "po2csv.h"
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <ctype.h>
 #include <errno.h>
 
 #define BUFFER_SIZE 4096
 #define MAX_LINE_LENGTH 8192
 
 typedef enum {
     STATE_NONE,
     STATE_MSGID,
     STATE_MSGSTR
 } ParserState;
 
 /**
  * Print usage information
  */
 void print_usage(const char *program_name) {
     printf("po2csv - Convert PO files to CSV\n");
     printf("Usage: %s <input.po> <output.csv>\n", program_name);
     printf("\n");
     printf("Options:\n");
     printf("  -h, --help    Display this help message\n");
 }
 
 /**
  * Escape special characters in CSV
  */
 int po2csv_escape_csv(const char *input, char *output, size_t output_size) {
     size_t i, j;
     int needs_quotes = 0;
     
     // Check if the string needs quotes (contains commas, quotes, or newlines)
     for (i = 0; input[i]; i++) {
         if (input[i] == ',' || input[i] == '"' || input[i] == '\n' || input[i] == '\r') {
             needs_quotes = 1;
             break;
         }
     }
     
     j = 0;
     
     // Add opening quote if needed
     if (needs_quotes && j < output_size - 1) {
         output[j++] = '"';
     }
     
     // Copy and escape the input
     for (i = 0; input[i] && j < output_size - 2; i++) {
         if (input[i] == '"') {
             // Double up quotes
             if (j < output_size - 2) {
                 output[j++] = '"';
                 output[j++] = '"';
             }
         } else {
             output[j++] = input[i];
         }
     }
     
     // Add closing quote if needed
     if (needs_quotes && j < output_size - 1) {
         output[j++] = '"';
     }
     
     // Null terminate
     output[j] = '\0';
     
     return j;
 }
 
 /**
  * Parse a quoted string from a PO file line
  * Returns a pointer to the allocated string (must be freed by caller)
  * or NULL on error
  */
 char *parse_quoted_string(char *line) {
     char *start, *end, *result;
     
     // Find the first quote
     start = strchr(line, '"');
     if (!start) {
         return NULL;
     }
     
     start++; // Skip past the quote
     
     // Find the closing quote
     end = strrchr(start, '"');
     if (!end || end == start) {
         return NULL;
     }
     
     // Allocate memory for the string
     result = (char *)malloc((end - start + 1) * sizeof(char));
     if (!result) {
         return NULL;
     }
     
     // Copy the string content
     strncpy(result, start, end - start);
     result[end - start] = '\0';
     
     return result;
 }
 
 /**
  * Process a continuation line (starting with ")
  * Appends the content to the given buffer
  * Returns 1 on success, 0 on failure
  */
 int process_continuation(char *line, char *buffer, size_t buffer_size) {
     char *content = parse_quoted_string(line);
     if (!content) {
         return 0;
     }
     
     // Check if there's enough space in the buffer
     if (strlen(buffer) + strlen(content) >= buffer_size - 1) {
         free(content);
         return 0;
     }
     
     // Append the content
     strcat(buffer, content);
     free(content);
     return 1;
 }
 
 /**
  * Convert a PO file to CSV format
  */
 int po2csv_convert(const char *input_file, const char *output_file) {
     FILE *in_fp, *out_fp;
     char line[MAX_LINE_LENGTH];
     char msgid[BUFFER_SIZE] = "";
     char msgstr[BUFFER_SIZE] = "";
     char escaped_msgid[BUFFER_SIZE * 2];
     char escaped_msgstr[BUFFER_SIZE * 2];
     ParserState state = STATE_NONE;
     int line_num = 0;
     char *content;
     
     in_fp = fopen(input_file, "r");
     if (!in_fp) {
         fprintf(stderr, "Error: Could not open input file %s: %s\n", 
                 input_file, strerror(errno));
         return 1;
     }
     
     out_fp = fopen(output_file, "w");
     if (!out_fp) {
         fprintf(stderr, "Error: Could not open output file %s: %s\n", 
                 output_file, strerror(errno));
         fclose(in_fp);
         return 1;
     }
     
     // Write CSV header
     fprintf(out_fp, "msgid,msgstr\n");
     
     while (fgets(line, MAX_LINE_LENGTH, in_fp)) {
         line_num++;
         
         // Remove trailing newline
         size_t len = strlen(line);
         if (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r')) {
             line[len-1] = '\0';
             if (len > 1 && line[len-2] == '\r') {
                 line[len-2] = '\0';
             }
         }
         
         // Skip empty lines and comments
         if (line[0] == '\0' || line[0] == '#') {
             continue;
         }
         
         // Check for msgid
         if (strncmp(line, "msgid ", 6) == 0) {
             // If we were processing a previous entry, write it out
             if (strlen(msgid) > 0) {
                 po2csv_escape_csv(msgid, escaped_msgid, sizeof(escaped_msgid));
                 po2csv_escape_csv(msgstr, escaped_msgstr, sizeof(escaped_msgstr));
                 fprintf(out_fp, "%s,%s\n", escaped_msgid, escaped_msgstr);
                 
                 // Reset buffers
                 msgid[0] = '\0';
                 msgstr[0] = '\0';
             }
             
             state = STATE_MSGID;
             content = parse_quoted_string(line);
             if (content) {
                 strncpy(msgid, content, BUFFER_SIZE - 1);
                 msgid[BUFFER_SIZE - 1] = '\0';
                 free(content);
             }
         }
         // Check for msgstr
         else if (strncmp(line, "msgstr ", 7) == 0) {
             state = STATE_MSGSTR;
             content = parse_quoted_string(line);
             if (content) {
                 strncpy(msgstr, content, BUFFER_SIZE - 1);
                 msgstr[BUFFER_SIZE - 1] = '\0';
                 free(content);
             }
         }
         // Continuation of a multi-line string
         else if (line[0] == '"') {
             if (state == STATE_MSGID) {
                 if (!process_continuation(line, msgid, BUFFER_SIZE)) {
                     fprintf(stderr, "Warning: Failed to process continuation line %d\n", line_num);
                 }
             } else if (state == STATE_MSGSTR) {
                 if (!process_continuation(line, msgstr, BUFFER_SIZE)) {
                     fprintf(stderr, "Warning: Failed to process continuation line %d\n", line_num);
                 }
             }
         }
     }
     
     // Write the last entry if there is one
     if (strlen(msgid) > 0) {
         po2csv_escape_csv(msgid, escaped_msgid, sizeof(escaped_msgid));
         po2csv_escape_csv(msgstr, escaped_msgstr, sizeof(escaped_msgstr));
         fprintf(out_fp, "%s,%s\n", escaped_msgid, escaped_msgstr);
     }
     
     fclose(in_fp);
     fclose(out_fp);
     
     return 0;
 }
 
 /**
  * Main function
  */
 int main(int argc, char *argv[]) {
     // Check for help option
     if (argc > 1 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
         print_usage(argv[0]);
         return 0;
     }
     
     // Check argument count
     if (argc != 3) {
         fprintf(stderr, "Error: Wrong number of arguments\n");
         print_usage(argv[0]);
         return 1;
     }
     
     // Run conversion
     int result = po2csv_convert(argv[1], argv[2]);
     
     if (result == 0) {
         printf("Successfully converted %s to %s\n", argv[1], argv[2]);
     }
     
     return result;
 }