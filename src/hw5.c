#include "hw5.h"

int main(int argc, char *argv[]) {

    //1: initialize variables
    //IN/OUTFILE: stores read file and file to be written to
    //Initialized to null in order to check if they exist
    FILE *infile = NULL;
    FILE *outfile = NULL;
    
    //Variables for getopt
    extern char *optarg;
    extern int optind;
    int c=0;
    //Represent whether we have seen each variable below
    int srec = 0, rrec=0,lrec=0,wrec=0;

    //Search text and replace text: represent the word being searched and the word to replace it with
    //Set to NULL, since they are needed, to check if they are missing
    char *search = NULL;
    char *rplcmt = NULL;

    //LINE NUMBERS
    //Represent line of infile to start reading and last line of infile to read
    //Set to default values, since they are optional
    unsigned long int startLine = 1;
    unsigned long int endLine = 0xffffffffffffffff;

    //Used to determine missing/invalid argument error code precedence
    /*If no argument is passed to one of the tags/no such tag for -s or -r exists, its corresponding space in tagnoarg will be filled
    0: -s
    1: -r
    2: -l
    In precedence order, they will be read to determine which error code to return
    */
    int tagnoarg[] = {0,0,0};

    if (argc<7){
        return MISSING_ARGUMENT;
    }

    while((c=getopt(argc,argv,"s:r:l:w"))!=-1){
        switch(c){
            case 's':
                if (srec == 1) return DUPLICATE_ARGUMENT;
                else{
                    srec = 1;
                    if (optarg[0]=='-') tagnoarg[0] = 1;
                    else search = optarg;
                }
                break;
            case 'r':
                if (rrec == 1) return DUPLICATE_ARGUMENT;
                else {
                    rrec = 1;
                    if (optarg[0]=='-') tagnoarg[1] = 1;
                    else rplcmt = optarg;
                }
                break;
            case 'l':
                startLine = 0;
                endLine = 0;
                if (lrec == 1) return DUPLICATE_ARGUMENT;
                else{
                    lrec = 1;
                    if (optarg[0]=='-') tagnoarg[2] = 1;
                    else{
                        //Otherwise, parse the string for start and end line numbers:
                        //separate ordered pair by commas
                        char *tok = strtok(optarg,",");
                        //iterator variable
                        int k = 0;
                        //while there are still tokens
                        while (tok != NULL){
                            //parse each successive token into a long; if there are more than 2 tokens drop them
                            if (!k) startLine = atol(tok);
                            else if (k<2) endLine = atol(tok);
                            else {
                                tagnoarg[2] = 1;
                                break;
                            }
                            k++;
                            tok = strtok(NULL,",");
                        }
                        if (startLine==0||endLine==0||startLine>endLine) tagnoarg[2] = 1;
                    }
                }
                break;
            case 'w':
                if (wrec == 1) return DUPLICATE_ARGUMENT;
                else wrec = 1;
                break;
            case '?':
                break;
        }
    }
    infile = fopen(argv[argc-2],"r");
        if (infile==NULL) {
            return INPUT_FILE_MISSING;
        }
    outfile = fopen(argv[argc-1],"w");
        if (outfile==NULL){
            return OUTPUT_FILE_UNWRITABLE;
        }

    if (infile==NULL) return INPUT_FILE_MISSING;
    else if (outfile==NULL) return OUTPUT_FILE_UNWRITABLE;

    if (tagnoarg[0]||!srec){ 
        fclose(infile);
        fclose(outfile);
        return S_ARGUMENT_MISSING;
    }
    else if (tagnoarg[1]||!rrec){
        fclose(infile);
        fclose(outfile); 
        return R_ARGUMENT_MISSING;
    }
    else if (tagnoarg[2]){ 
        fclose(infile);
        fclose(outfile);
        return L_ARGUMENT_INVALID;
    }
    //NOW: in the absence of errors up to this point, read and write the file:
    
    //Step 1: just copy all parts of the file that are before the first line to be read
    for (long unsigned int i = 1; i<startLine;i++){
        char lineArr[MAX_LINE];
        char *line = fgets(lineArr,MAX_LINE,infile);
        if (line!=NULL) fputs(line,outfile);
        else{//if startline is greater than file length, just copy the file, then close in/outfile and return
            fclose(infile);
            fclose(outfile);
            return 0;
        }
    }

    int searchlen = strlen(search);
    int replen = strlen(rplcmt);
    
    //CASE 1: wildcard is enabled
    if (wrec){
        char *first_ast = strchr(search,'*');
        char *last_ast = strrchr(search,'*');
        //ensure valid wildcard parameter: only one asterisk, either at the start or at the end
        if (first_ast==NULL){
            fclose(infile);
            fclose(outfile);
            return WILDCARD_INVALID;
        }
        else if (((first_ast==search)||(first_ast==(search+(strlen(search)-1))))&&first_ast==last_ast){
            int direction=0;//records which direction the wildcard searches: 0 -> words beginning in <search>, 1 -> words ending in <search>
            if (search[0]=='*') direction = 1;
            
            //Remove the asterisk; we will use var direction to track which side to use from now on
            if (direction) search = search+1;
            else *(search+searchlen-1) = '\0';
            searchlen = searchlen - 1;
            
            /*
            BASIC OUTLINE: For each line between startLine and endLine (inclusive):
            a. If end of file is reached, break immediately and do nothing
            b. Otherwise, continue:
            1. Initialize a char[] line using fgets of infile
            2. Initialize an iterator value to 0
            3. While *(line + iter)!='\0':
                1. Set pointer end to first location of the search string (minus asterisk)
                2. If direction, search backwards from end until reaching punctuation or a space (or a null terminator);
                this will be the index iterated up to before inserting the replacement text
                Also, check the space immediately after end+searchlen-1 -> if it is not a space or punctuation mark, skip
                    - If end == line, set it to 0
                3. If !direction, search forward from (end+searchlen-1) until reaching a punctuation, space, or \0; save the number of
                iterations up to that point to increment iter, then iterate up to its beginning, insert replacement text, and increment iter
                Also, search the space immediately before end -> if it is not a space or punctuation mark, skip
                    - If immediately after end is a null terminator, it should already be handled; the following pass through the loop will
                    immediately break because search+searchlen-1 = '\0'
            */

            for (long unsigned int i = startLine; i<=endLine; i++){
                    char lineArr[MAX_LINE];
                    char *line = fgets(lineArr,MAX_LINE,infile);
                    if (line==NULL){
                        break;
                    }
                    int iter = 0;

                    while (*(line+iter)!='\0'){
                        char *end = strstr(line,search);
                        //determine number of characters in the word
                        if (end==NULL){
                            fputs(line,outfile);
                            break;
                        }
                        else if (direction){
                            //Search backwards
                            if (!(isspace(*(end+searchlen))||
                            ispunct(*(end+searchlen))||
                            *(end+searchlen)=='\0')){//If it's not actually the end of a word, skip
                                while ((line+iter)!=(end+searchlen)){
                                    fputc(*(line+iter),outfile);
                                    iter++;
                                }
                                line = end + searchlen;
                                iter = 0;
                            }
                            //CASE 2: IT IS ACTUALLY THE END OF A WORD
                            else{
                                int beforeEnd = 0;
                                while ((end-beforeEnd)!=line){
                                    char check = *(end-beforeEnd);
                                    if (isspace(check)||ispunct(check)){ 
                                        beforeEnd--;
                                        break;
                                    }
                                    else beforeEnd++;
                                }
                                while ((line+iter)!=(end-beforeEnd)){
                                    fputc(*(line+iter),outfile);
                                    iter++;
                                }
                                fputs(rplcmt,outfile);
                                line = end + searchlen;
                                iter = 0;
                            }
                        }else {
                            //Search forwards
                            if ((end!=line)&&(!(isspace(*(end-1))||
                            ispunct(*(end-1))||
                            *(end-1)=='\0'))){//If it's not actually the beginning of a word, copy everything down to the end of the searchtext
                                while ((line+iter)!=(end+searchlen-1)){
                                    fputc(*(line+iter),outfile);
                                    iter++;
                                }
                                line = end + searchlen-1;
                                iter = 0;
                            }
                            //CASE 2: it is the beginning of a word
                            else{
                                int afterEnd = 0;
                                while(*(end+(searchlen)+afterEnd)!='\0'){
                                    char test = *(end+searchlen+afterEnd);
                                    if (isspace(test)||ispunct(test)||test=='\0') break;
                                    afterEnd++;
                                }
                                while((line+iter)!=end){
                                    fputc(*(line+iter),outfile);
                                    iter++;
                                }
                                fputs(rplcmt,outfile);
                                line = (line + iter + searchlen + afterEnd);
                                iter = 0;
                            }
                        }
                    }
            }

        }else{
            fclose(infile);
            fclose(outfile);
            return WILDCARD_INVALID;
        } 
    }
    //CASE 2: simple search-and-replace
    else{
        for(long unsigned int i = startLine; i<=endLine;i++){
            //once end of file is reached, break the loop
                char lineArr[MAX_LINE];
                char *line = fgets(lineArr,MAX_LINE,infile);
                if (line==NULL){
                    break;
                }
                /*BASIC OUTLINE:
                1. Get the line and copy it into str line
                2. Use strstr to find the first instance of the search word
                3. Copy all characters in line up to that point into outfile
                4. Copy the replacement text into outfile
                5. Set line = substr of line starting after the end of the search word
                6. Repeat until you reach \0
                */
                int iter = 0;
                while (line[iter]!='\0'){
                    //Pointer to the first occurrence of search
                    char *end = strstr(line,search);

                    //if there is no occurrence, copy the line and skip entirely
                    if (end==NULL){
                        fputs(line,outfile);
                        break;
                    }

                    //Copy up to the point at which the address of the char line[iter]
                    while(&(line[iter]) != end){
                        fputc(line[iter],outfile);
                        iter++;
                    }
                    //Once reaching that point, simply copy the replacement text into outfile, then skip
                    //the search text by incrementing iter
                    for (int k = 0; k<replen; k++){
                        fputc(rplcmt[k],outfile);
                    }
                    iter+=searchlen;

                    //set line to point to the address iter ahead of itself
                    line = (line+iter);
                    //reset iter to 0 to start again with shortened line, repeat until reaching the end
                    iter = 0;
                }
        }
        
    }
    //If not at the end of the file, copy remaining lines of infile to outfile
    while (!feof(infile)){
        char lineArr[MAX_LINE];
        char *line = fgets(lineArr,MAX_LINE,infile);
        fputs(line,outfile);
    }
    //close streams and return success
    fclose(infile);
    fclose(outfile);
    return 0;
}
