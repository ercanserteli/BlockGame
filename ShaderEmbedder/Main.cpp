#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <string>

using namespace std;

FILE* fp_in = nullptr;
FILE* fp_out = nullptr;

string in_file;
string out_file;
string array_name;

void out(const char* format, ...) {
    char print_buffer[1024];
    va_list list;

    va_start(list, format);

    vsnprintf(print_buffer, 1024, format, list);

    va_end(list);

    fprintf(fp_out, "%s", print_buffer);
}

void make_c_string(string& in) {
    string out;

    for (const char c : in) {
        if ('"' == c)
            out += "\\\"";
        else if ('\\' == c)
            out += "\\\\";
        else
            out += c;
    }
    in = out;
}

void write_array_header() { out("const char * %s =\n", array_name.c_str()); }

void write_array_footer() { out(";\n"); }

void write_line(const string& line) { out("\"%s\\n\"\n", line.c_str()); }

int main(int argc, char** args) {
    if (argc != 4) {
        printf("syntax error, usage :  array_name infile outfile");
        exit(0xff);
    }

    array_name = args[1];
    in_file = args[2];
    out_file = args[3];

    errno_t result = fopen_s(&fp_in, in_file.c_str(), "rt");
    if (result == 0) {
        result = fopen_s(&fp_out, out_file.c_str(), "wt");
        if (result == 0) {
            out("#pragma once\n\n");

            write_array_header();

            char buff[1024];
            while (fgets(buff, sizeof(buff), fp_in)) {
                string s(buff);
                s = s.substr(0, s.find('\n'));

                make_c_string(s);

                write_line(s);
            }
            write_array_footer();

            fclose(fp_out);
        } else {
            printf("Error opening %s\n", out_file.c_str());
            exit(0xff);
        }
        fclose(fp_in);
    } else {
        printf("Error opening %s\n", in_file.c_str());
        exit(0xff);
    }
}
