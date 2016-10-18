#include <bits/stdc++.h>
using namespace std;



/*PROTOTYPE OF CLASS VALUE*/
class value; 

/* SOME SHORT DEFINITIONS*/

typedef vector <value> vl;
typedef vector <vl> vll;
typedef vector<string> vs;
typedef vector<vs> vss;
typedef pair<char,string>  ps;

/*******************************************DATA STRUCTURES USED***********************************************/





// tableA, tableB are two vectors which represent relations where all the operations will be perforomed.
// These are arranged in a manner such that tuples of a relation will become column of a 2D vector 
// this representation is useful in efficient retrieval of attributes by mapping attribute to indices of vector. 
vll tableA,tableB;

// table_opeartes = 1 denotes tableA is currently in use.
// table_operates = 2  denotes tableB is active.
int table_operates = 1;


/***********************************************SOME HASH TABLES************************************************

// hash_tableA  ->  maps attribute names of tableA to their respective positions in tableA.
// hash_tableB  ->  maps attribute names of tableB to their respective positions in tableB.
// mytables     ->  maps names of relations to vectors representing them . 
// rename_hashA ->  maps modified attribute names(names with attribute name prefixed with "filename.")of tableA 
				    to original attribute names of tableA. 
// rename_hashB ->  maps modified attribute names(names with attribute name prefixed with "filename.")of tableB 
				    to original attribute names of tableB
****************************************************************************************************************/
map<string,int> hash_tableA, hash_tableB;
map<string,string> rename_hashA,rename_hashB;
map<string,vll *> mytables; 



/*FUNCTIONS PROTOTYPE */
int pre_parser(string q);
int parser(string q);
int execute (stack<ps> &stk);
int fill_table(string filename);
int select(string condition);
int project(string condition);
int rename(string a,string old_file_name);
int cross_product(string file1,string file2);
int set_difference(vll &tableA,vll &tableB);
int set_union(vll &tableA,vll &tableB);
void display_trans(vll &v);
void display_hash(map<string,int> &table);
void eval(string query,string &val1,string &val2,string &oper);
void clear_containers();
void  display_result(vll &v,string name);
template<typename T>
bool compare(T val1,T val2,string oper);
int check_balanced_paraenthesis(string query);





/*
// class value -> we store our data in a object of class value.
// i.e each value of table is stored in a object of class value.
// This is done so that our vectors tableA,tableB can hold multi datatypes. 
*/
class value
{

   public:

	/*************************DATA MEMBERS USED IN CLASS value*****************************

	    Data member 'type' denotes the type of value held by the object of class value.
	    type = 1 -> Integer value is stored in object.
	    type = 2 -> A floating value is stored in object.
	    type = 3 -> A string  value is stored in object.

	 *********************************************************************************
	*/

	int type = -1;
	int idata;
	string sdata;
	double ddata;


	/*
	// We overload the constructor by varying the type of data.
	*/

	value(int data)
	{
		idata = data;
		type  = 1;	
	}

	
	value(double data)
	{
		ddata = data;
		type  = 2;	
	}

	value(string data)
	{
		sdata = data;
		type  = 3;	
	}


	/*
		// we overload operator '==' to compare two objects.
		// two objects can be compared if they hold the same data type.

	*/

	 friend bool operator ==(const value &a, const value &b) {


       
        if (a.type == 1)//check if the object holds an integer value 

        {

        	return a.idata == b.idata;
        }
        else if (a.type == 2)//check if the object holds a floating point value
        {
        	
        	return a.ddata == b.ddata;
        }
        else  //the object holds a string value
        {
        
        	return a.sdata == b.sdata;
        }
    }
};



/* structure cmp  is used to compare two vectors that have objects of class value as its elements*/

struct cmp {
    bool operator ()(const vl &a, const vl &b) {
        
        bool flag = true;//Initially  we assert that two objects are same. 

      
        for (int i = 0; i < a.size(); ++i)
        {
        	flag = flag and a[i]==b[i];// Check if ith object in vector a is same as ith object of vector b
        }
        return not flag;
    }
};



/*
*************************CLASS Table*******************************
class Table is used to represent the two files that conatins the data.
Each object's trans_table is used to represent a relation.
*/

class Table{
	/*
		************************DATA MEMBERS USED IN CLASS Table**********************


		name        -> will hold name of file from where data will be read
        attributes  -> no. attributes.
        records 	-> no. of tuples.
        main_table  -> read data is stored in this vector named main table.
        trans_table -> Data is stored in this vector in a form such that each tuple 
        			   of relation is now a column of this vector. 
        trans_hash  -> hash table to map name of attributes to index of trans_table
        			   as each attribute is placed in a row of trans_table.
       **************************************************************************
	*/
    public:
        string name; 
        int attributes = 0;  
        int records = 0;     
        vss main_table; 
        vll trans_table; 
        vector<string> dtype;
        map<string, int > trans_hash;

        /*
		constructor creates main_table,trans_table,trans_hash,counts attributes and tuples and displays the data
		read from file.
        */
        Table(string filename)
        {
            name = filename;

            create_main_table();
            count_rows_cols();
            create_trans_table();
            create_trans_hash();

            //we send filename without extension,hence we extract substring from it .
            display_result(trans_table,name.substr(0,name.find(".")));
           
        }

        // counts no of attributes and no of records in table.
        void count_rows_cols()
        {
            attributes = main_table[0].size();
            records    = main_table.size();
        }


    // Reads data from file and stores it in main_table.
    void create_main_table()
    {
        const char *myfile = name.c_str();// get the file name from data member 'name' 
        ifstream  data(myfile);
        string line;

        /*
          In the first row of file we some numbers which represent domain of attribute.
		  1-> integer
		  2-> floating point value
		  3-> string value
        */

        int i=0,j=0,k=0;
        while(getline(data,line))// read a line from file
        {
            stringstream  lineStream(line);
            string  cell;
            vs temp;
            while(getline(lineStream,cell,','))// iterate each value of a row seperated by ','
            {	
                temp.push_back(cell);
                if (k ==0) // when k = 0 ,we read the numbers representing data types in vector dtype . 
                	dtype.push_back(cell);

            }
            k++;
            main_table.push_back(temp);
            i++;
        }

    }

    /*
    // create vector trans_table.
    // Transforms data from main_table to trans_table such that values of each attribute will 
       be stored in seperate rows of trans_table.
    // Each value in trans_table will be object of class value.
    // Name of attributes are stored in first column of this trans_table.
    */

    void create_trans_table()
    {

        for(int i=0 ;i< attributes;i++)
        {
             vl temp;
            for (int j=1; j< records;j++)
            {

            	if (dtype[i] == "1" and j>=2)// check if value is integer
                	temp.push_back(value(atoi(main_table[j][i].c_str())));//convert string to inetger and create object of class value and push in temp
                
                 else if (dtype[i] == "2" and j>=2)// check if value is float
                 	temp.push_back(value(atof(main_table[j][i].c_str())));//convert string to float and create object of class value and push in temp

                 else
                 	temp.push_back(value(main_table[j][i].c_str()));//we have string and create object of class value and push in temp
            }
            trans_table.push_back(temp);
        }
    }


    // Creates tarns_hash map which maps attribute names to their position indices of the trans_table.
    void create_trans_hash()
    {
        for (int i=0;i<trans_table.size();i++)
        {
            trans_hash[trans_table[i][0].sdata] = i ;// map attribute name to index 
            trans_hash[name+"."+trans_table[i][0].sdata] = i;// map attribute name prefixed with "filename."
        }
    }
   
};


/* Driver program */
int main()
{
 		
    	cout <<  "**********************************************RELATION ALGEBRA QUICK GUIDE***************************************************";
    	cout << endl;
    	cout << endl;
    	cout << "1. @: PROJECT";
    	cout << endl;
    	cout << "2. #: RENAME";
    	cout << endl;
    	cout << "3. $: SELECT";
    	cout << endl;
    	cout << "4. *: CARTESIAN PRODUCT";
    	cout << endl;
    	cout << "5. -: SET DIFFRENCE";
    	cout << endl;
    	cout << "6. +: SET UNION";
    	cout << endl;
    	cout << "7. [] : SPECIFY ATTRIBUTES IN PROJECT AND SELECT";
    	cout << endl;
    	cout << "8. {}: SPECIFY RENAMED ATTRIBUTES IN REANAME";
    	cout << endl;
    	cout << "9. REFERENCE QUERY: @[DEPARTMENT, ROLL](@[STUD_NAME,STUD_ROLL,STUD_DEPT](#[x{STUD_NAME,STUD_ROLL,STUD_DEPT}](student)))";
    	cout << endl;
    	cout << endl;
    	
    	cout << "*******************************************************START*****************************************************************";
    	cout << endl;

    	/*Create two objects of class Table T1 and T2.
		 T1 - > object for student.csv.
		 T2 - > object for instructor.csv.
    	*/
        Table T1("student.csv"),T2("instructor.csv");
        
    while(1)
    {	
    	
    	// Make entries in mytables hash map by mapping names of relations to their corresponding object's trans_table.
 		mytables["student"] = &T1.trans_table;
 		mytables["instructor"] = &T2.trans_table;
 		//Variabe query will be used to input query from user.
    	string query;
    	cout <<"RA>>>"; 
        getline(cin, query);
        
        if (query == "exit")
        {	cout << endl;
        	cout << "***************************************************STOP******************************************************************";
        	cout << endl;
        	break;
        }

        //Pass the query to pre_parser and dispaly the result if no error occurs i.e when status is not -1.
        int status = pre_parser(query);
        if (status!=-1)
 			display_result(tableA,"");
        cout << endl;
        // clear all the containers and get ready for next query.
        clear_containers();
    	
    }

}



/**********************************pre_parser function***************************************************

pre_parser functions checks if there is operator '-' for set difference or '+' for set_union.
If any of these two operators is present in the query, we split the query q,in two parts q1,q2.
	q1 ->  contains part of q before '-' or '+'.
	q2 ->  contains part of q after '-' or '+'.
	then pass these queries q1,q2 to parser one by one.
If it does not contains '-' or '+' we pass the entire query to parser.
variable status -> indicates status of query when passed parser.
status = 1 -> query got executed successfully.
status = -1 -> there was error while parsing the query.


********************************************************************************************************
*/
int  pre_parser(string q)
{
	int status = 1;
	string::iterator end_pos = remove(q.begin(), q.end(), ' ');
	q.erase(end_pos, q.end());

	int diff  = q.find("-"); // check if query contains'-'.
	int uni   =  q.find("+");// check if query contains'+'.

	if (diff!=string::npos) // if quey contains '-'

	{

		//split query q in q1 and q2
		string q1 = q.substr(0,diff);
		string q2 = q.substr(diff+1,q.length()-1);

		//send q1 to parser
		status = parser(q1);
		if (status == -1)
			return status;
		//Make tableB currently active
		table_operates = 2;

		//send q2 to parser
		status = parser(q2);
		if (status == -1)
			return status;
		status = set_difference(tableA,tableB);
		if (status == -1)
			return status;
	}

	else if(uni!=string::npos)// if query contins '+'
	{
		//split query q in q1 and q2
		string q1 = q.substr(0,uni);
		string q2 = q.substr(uni+1,q.length()-1);

		//send q1 to parser
		status = parser(q1);
		if (status == -1)
			return status;
		//Make tableB currently active
		table_operates = 2;

		//send q2 to parser
		status = parser(q2);
		if (status == -1)
			return status;

		 status = set_union(tableA,tableB);
		 if (status == -1)
			return status;
	}

	else {

		// if query does not contain '-' or '+' pass query q to parser
		status = parser(q);

		if (status == -1)
			return status;
	
	}
	// if everything goes on smoothly return 1
	return 1;

}


/**********************************parser function****************************************************************

->  parser function removes spaces in query q if any.
-> It handles select,project,rename queries.
-> It parses the query q and puts a pair in stack.A pair which has a symbol as its first element.
	each symbol denotes the type of operation i.e select or project or rename.Second element of the pair
	contains the assosciated conditions with each operations.On the top of stack it conatins pair having 'f' symbol 
	and name of table in the second element of the pair.
	Example:
	       ('f',relationname)
	       ('@',conditions) -> project with assosciated attributes.
	       ('$',conditions) -> select with assoscited conditions.
	       ('#',condtions)  -> rename with assosciated new names.
->  The whole stack is passed to execute function for further execution.
-> Returns 1 if everything runs fine else -1. 

******************************************************************************************************************/

int  parser(string q)
{	
	int status =1;
	

	//Remove spaces from query
	string::iterator end_pos = remove(q.begin(), q.end(), ' ');
	q.erase(end_pos, q.end());

	status = check_balanced_paraenthesis(q);
	if (status == -1)
		{
			cout << endl;
			cout <<"*************ERROR*************************" << endl;
			cout <<"          PARANTHESIS NOT BALANCED   " << endl;
			cout <<"*************ERROR*************************" << endl;
			cout << endl;
			return -1;

		}


	
	//parse query and match the operation and make a pair of (symbol,condtions) and push it in stack.
	string temp ="";
	stack<ps> stk;
	for(int i=0 ;i<q.length();i++)
	{
		if (q[i] == '@' or q[i] == '$' or q[i] == '#')
		{
			switch (q[i])
			{
				case '@':
					i++;
					while (q[i]!=']')
					{
						temp += q[i];
						i++;
					}
					temp += "]";
					stk.push(make_pair('@',temp));//make pair of project operation and its assosciated conditions
					temp ="";
					break;
				
				case '$':
					i++;
					while (q[i]!=']')
					{
						temp += q[i];
						i++;
					}
					temp += "]";
					stk.push(make_pair('$',temp));//make pair of select operation and its assosciated conditions
					temp ="";
					break;	
					
					
				case '#':
					i++;
					while(q[i]!=']')
					{
						temp += q[i];
						i++;
					}
					
					temp += "]";
					stk.push(make_pair('#',temp));//make pair of rename operation and its assosciated conditions
					temp ="";
					break;
			}
			
			}
			else if (q[i] == '(' and q[i+1] !='(')
			{
				
				if(q[i+1] >=97 and q[i+1]<=122 or (q[i+1] >=65 and q[i+1] <=90) or q[i+1]=='*')
				{
				
					while(q[i]!=')')
					{
						temp += q[i];
						i++;
					}
					temp+=")";
					
					stk.push(make_pair('f',temp));//push the last pair ('f',filename) in the stack.
					temp = "";
				}
			
				
			}
			
			else
			{
				i++;
			}
		
		}
	
	//parse the stack to execute function
	status = execute(stk);
	if (status == -1)
		return status;
	return 1;
	

}


/************************************************execute function*************************************************
-> execute function pops each pair from stack and performs correct operation according to symbol in pair.
-> Since the top of stack always contains pair with 'f' symbol and name of table.
	we first pop the top of stack and see if the table name contains *(cross product)
	if it does ,this means we have two tables to operate now,we extract names of both the tables
	and call the function cross_product with both table names as its arguments.

	if the table name does not contain '*' we call fill_table with table name as its arguments. 

-> After popping the top pair pair we enter a while loop until our stack becomes empty.
	with each pair popped from stack, we match symbol of each pair with the corresponding opeartion
	functions using switch case and pass the assosciated conditions as arguments to the functions.
	'@' - project(condition)
	'$' - select(condition)
	'#' - rename(condition,filename)

*****************************************************************************************************************/



int  execute (stack<ps> &stk)
{
	int status = 1;
	
	ps p = stk.top();
	string str= p.second;
	string filename = str.substr(1,str.length()-2);

	int cross = filename.find("*");// check if the file name contains cross product symbol '*'
	if (cross != string::npos)// if it does contain '*' 
	{
		//get the names of two relations in file11 and file2.
		string file1 = filename.substr(0,cross);
		string file2 = filename.substr(cross+1,filename.length()-1);

		//perform cross product
		status = cross_product(file1,file2);
		if (status == -1)
			return status;
	 }
	else// if query does not contain *
	  status = fill_table(filename);

	if (status == -1)
		return status;

	
	stk.pop();// remove top pair from stack which contained filename 



	//pop a pair and perform corresponding operation by matching symbol from pair to different case statements.
	while (!stk.empty())
	{
		
		p = stk.top();
		char operation = p.first;
		string condition = p.second;
		stk.pop();
		
		switch(operation)
		{
		
			case '@':
				status = project(condition);
				if (status == -1)
					return status;
				break;

			case '$':
				status = select(condition);
				if (status == -1)
					return status;
				break;
				
			case '#':
				
				 status = rename(condition,filename);
				 if (status == -1)
				 	return status;
				
				break;
				
			
		}
	}

	return 1;
	
}


/*******************************************fill_table function*********************************************************

-> Function fill_table fills the currently active table with trans_table of Table object.
-> If the table exists then it fetches the Table object with the given filename and duplictaes the
	data from its trans_table to the currently active table(tableA or tableB) depending on table_operates value.
-> It also prepare current hash_table(hash_tableA or hash_tableB) and current rename hash (rename_hashA or rename_hashB).
-> Current hash_table is prepared by extracting attribute names from first column of trans_table and 
	mapping them to correct positions in the current active table.Modified attribute names 
	(attributes accesible from name of table ) are also mapped to correct index of current active table.
-> Current rename hash is prepared by mapping modified attributes i.e attributes with "table_name." prefix 
	to actual name of attributes.
	Example : student.name ->  name
-> Return 1 if everything gets executed successfully else return -1.

****************************************************************************************************************************/

int  fill_table(string filename)

{	


	map<string,string>::iterator it;

	//fill_table checks if table with given filename exists or not.
	if (mytables.find(filename) == mytables.end())
	{
		cout << endl;
		cout <<"*************ERROR*********************" << endl;
		cout <<"         RELATION "+filename+" NOT FOUND        "<<endl;
		cout <<"*************ERROR*********************" << endl;
		cout << endl;
		return -1;

	}

	vll *current;
	map<string,int > *mp;
	map<string,string> *rnmhash;

	// obtain pointer to current active table,current hash table and current rename hash.

	if (table_operates == 1)// if tableA is currently active
	{
		current = &tableA;
		mp      = &hash_tableA;
		rnmhash = &rename_hashA;


	}
	else// if tableB is currently active
	{
		current = &tableB;
		mp      = &hash_tableB;
		rnmhash = &rename_hashB;
	}

	// obtain references to current active table,current hash table,current rename table.

	vll &out_table = *current;
	map <string ,int > &hash_out = *mp;
	map <string,string>  &curr_rnm = *rnmhash;
	vll &target_table = *mytables[filename];


	//map attribute names to indices in current active table.
	for(int i=0 ; i< target_table.size(); i++)
	{
		out_table.push_back(target_table[i]);
		hash_out[target_table[i][0].sdata] = i;
		hash_out[filename+"."+target_table[i][0].sdata] = i;
		curr_rnm[filename+"."+target_table[i][0].sdata] = target_table[i][0].sdata;

	}

	
	return 1;


}

/******************************************project function******************************************************************

-> selects certain attributes from currently active table.
-> collects attributes to be projected in a queue.
-> checks if the attributes mention in the query are valid or not by checking their existence in the current hash.
-> First it copies the entire current table in another vector.
-> clears current active table.
-> It copies back only the projected atrributes to current active table.
-> It also changes/modifies the current hash and current rename hash since the indices of the attributes would have changed
   in the trans_table.

*****************************************************************************************************************************/

int  project(string condition)
{
	
	queue<string> q;
	string temp = "";
	

	//collects attributes to be projected in a queue.
	for(int i=1;i<condition.length();i++)
	{
		while(condition[i]!=',' and condition[i]!=']')
		{
			temp += condition[i];
			i++;
		}

		q.push(temp);
		
		temp = "";
	}
	
	vll * current ;
	map<string,int> *mp;
	map<string,string> *rnmhash;

	// obtain pointer to current active table,current hash table and current rename hash.
	if (table_operates == 1)// if tableA is currently active
	{	
		
		current = &tableA;
		mp      = &hash_tableA;
		rnmhash = &rename_hashA;
	}
	else// if tableB is currently active
		{
	
		current = &tableB;
		mp      = &hash_tableB;
		rnmhash = &rename_hashB;
		}

	// obtain references to current active table,current hash table,current rename table.

	vll &current_table = *current;
	map<string,int> &current_hash = *mp;
	map<string,string> & curr_rnm = *rnmhash;
	vll temp_table;



	

	while (!q.empty())
	{
		//check if the attributes mention in the query are valid or not by checking their existence in the current hash
		if(current_hash.find(q.front()) == current_hash.end())
		{
			cout << endl;
			cout <<"*************ERROR*************************" << endl;
			cout <<"         ATTRIBUTE "+q.front()+" NOT FOUND        "<<endl;
			cout <<"*************ERROR*************************" << endl;
			cout << endl;
			return -1;
		}
		temp_table.push_back(current_table[current_hash[q.front()]]);
		q.pop();

	}

	current_table.clear();


	// copy back temp to current table.
	for (int i = 0; i < temp_table.size(); ++i)
	{
		current_table.push_back(temp_table[i]);
	}


	//upadte current hash
	for (int i = 0; i < current_table.size(); ++i)
	{
		current_hash[current_table[i][0].sdata] = i;
	}

	//update current hash by iterating in current rename hash
	map<string,string>::iterator it;
	for ( it = curr_rnm.begin(); it != curr_rnm.end(); ++it)
	{
		current_hash[it->first] = current_hash[curr_rnm[it->first]];
	}

	return 1;
}

/***************************************rename function*************************************************************

-> renames current active table ,attributes in current active tables with new names in the query.
-> Extracts new table name from string 'a' in variable named new_table_name ;
-> All the new names of attributes are extracted in queue. 
-> Checks if new names of  all atrributes are specified in query.
-> It updates mytables hash to map new name of table to currently active table.
-> It updates current hash to include mappings of new names to their corresponding indices in the current active table.
-> It updates current rename hash to include mappings from new names to original names in the current active table and also 
	modified names (new names prefixed with "new_table_name.") to original names.
-> If there occurs an error function returns -1,otherwise it returns 1.

*****************************************************************************************************************/

int rename(string a,string old_file_name)
{
	
	//Extracts new table name from string 'a' in variable named new_table_name
	// All the new names of attributes are extracted in queue q.
	string new_table_name = "",temp = "";
	queue<string> q;
	for(int i=1 ;i<a.length();i++)
	{
		while (a[i]!='{' and a[i]!=']')
		{	
			new_table_name += a[i];
			i++;
		}
		
		if (a[i]=='{')
		{
			i++;
			while (a[i]!=']')
			{
				while (a[i]!=',' and a[i]!='}')
				{   
					temp += a[i];
					i++;
				}
				i++;
				if (temp!= "")
					q.push(temp);
				temp = "";
			}
		
		}
		
	}
	
	if (new_table_name == "")
	{

		cout << endl;
		cout <<"**********************NAME ERROR**************" << endl;
		cout <<"         ENTER A VALID RELATION NAME       "<<endl;
		cout <<"**********************NAME ERROR**************" << endl;
		cout << endl;
		return -1;

	}
	
	
	vll *current;
	map<string,int> *mp;
	map<string,string> *rnmhash;
	// obtain pointer to current active table,current hash table and current rename hash.
	if (table_operates == 1)// tableA is active
	{  
		current = &tableA;
		mp      = &hash_tableA;
		rnmhash = &rename_hashA;
	}

	else{// tableB is active
		current = &tableB;
		mp      = &hash_tableB;
		rnmhash = &rename_hashB;

	}

	// obtain references to current active table,current hash table,current rename table.
	vll &current_table = *current;
	map<string,int> &current_hash = *mp;
	map<string,string> &curr_rnm =  *rnmhash;


	if (!q.empty())
	{

	//Checks if new names of all atrributes are specified in query
	if (q.size()!=current_table.size())
	{
		cout << endl;
		cout <<"**********************NUMBER MISMATCH ERROR**************" << endl;
		cout <<"         NUMBER OF ARGUMENTS IN RENAME QUERY INCORRECT        "<<endl;
		cout <<"**********************NUMBER MISMATCH ERROR**************" << endl;
		cout << endl;
		return -1;
	}


	}

	//It updates mytables hash to map new name of table to currently active table.
	mytables[new_table_name] = current;




	//It updates current hash to include mappings of modified  names to their corresponding indices in the current active table.	
	for (int i = 0; i < current_table.size(); ++i)
		{
			current_hash[new_table_name+"."+current_table[i][0].sdata] = i;
			curr_rnm[new_table_name+"."+current_table[i][0].sdata] = current_table[i][0].sdata;
		}	
	

	//It updates current rename hash to include mappings from new names to original names in the current active table and also 
	//modified names (new names prefixed with "new_table_name.") to original names.
	if (!q.empty())
	{	
		for (int i = 0; i < current_table.size(); ++i)
		{
			current_hash[q.front()] = i;
			current_hash[new_table_name+"."+q.front()] = i;
			curr_rnm[q.front()] = current_table[i][0].sdata;
			curr_rnm[new_table_name+"."+q.front()] = current_table[i][0].sdata;
			q.pop();
		}

	}

	return 1;
	
	
}

void display_trans(vll &v)
    {

    	for (int i = 0; i < v.size(); ++i)
    	{
    		for (int j = 0; j < v[i].size(); ++j)
    		{
    			if (v[i][j].type == 1)
    				cout << v[i][j].idata;
    			else if (v[i][j].type  == 2)
    				cout << v[i][j].ddata;
    			else 
    				cout << v[i][j].sdata;

    			cout << "   ";
    		}

    		cout << endl;
    	}

    }


void display_hash(map<string,int> &table)
{

	map<string,int>::iterator iit;
	 for (iit = table.begin(); iit !=table.end(); ++iit)
	{
		cout << iit ->first <<"  " << iit->second<<endl;
	}
}

/********************************************select function*******************************************************

-> selects tuples from the currently active table which satisfy the assoscited conditions. 
-> Can handle only conditions joined with '^' (and).
-> Parses the query ,extracts all the conditions joined with '^' and operator and pushes them in a vector.
-> For each condition it extracts the two operands and one operator(=,!=,>=,>,<,<=).
   The lhs operand has to be an attribute name from  while lhs operand may be an attribute name or it may be just a value.
-> With the help of helper function it finds the lhs operand value,rhs operand value and the operator value.
->  Iterates through each column of current active table each of which represent a tuple.Using compare function
	it sees if the tuple satisfies all the conditions and if it does,we push the index of column in a vector named index_store.
-> We copy the contents of the current active table in a secondary table .clear the active table.
	Copy only those tuples which qualified in index_store.
-> we also check if there occurs a type mismatch .example when you try to compare a string to integer.
-> If everything runs smoothly we rreturn 1 else return -1.

*******************************************************************************************************************/


int select(string condition)
{
	
	
	vector<string> subq;
	string temp="";
	int andor= 1;
	//Parses the query ,extracts all the conditions joined with '^' and operator and pushes them in a vector.

	if (condition.find("|")!=string::npos)
		andor = 2;
	

	for(int i=1 ; i< condition.length();i++)
	{
		while((condition[i]!='^' and condition[i]!=']') and (condition[i]!='|' and condition[i]!=']'))
		{
			temp += condition[i];
			i++;
		}
		
		subq.push_back(temp);
		temp= "";
	}


	vll *current;
	map<string,int> *mp;
	map<string,string> *rnmhash;

	// obtain pointer to current active table,current hash table and current rename hash.
	if (table_operates == 1)// tableA is active
	{  
		current = &tableA;
		mp      = &hash_tableA;
		rnmhash = &rename_hashA;
	}

	else{// tableB is active
		current = &tableB;
		mp      = &hash_tableB;
		rnmhash = &rename_hashB;

	}

	// obtain references to current active table,current hash table,current rename table.
	vll &current_table = *current;
	map<string,int> &current_hash = *mp;
	map<string,string> &curr_rnm =  *rnmhash;



	
	vector<int> index_store;
	index_store.push_back(0);

	// iterate through each tuple store index of tuple which satisfies all conditions in select clause.
	for(int j=1 ; j <current_table[0].size(); j++)
	{
	
		bool flag;
		if (andor == 1)
			flag = true;
		else
			flag = false;
		
		for(int i=0; i < subq.size();i++)
		{	
			string val1="",val2="",oper="",scol1,scol2;
			int icol1 = -999,icol2 = -999;
			double dcol1 = -999 ,dcol2 = -999;
			//For each condition it extracts the two operands and one operator(=,!=,>=,>,<,<=).
			//operands value are in val1 and val2.
			eval(subq[i],val1,val2,oper);
			

			if(current_hash.find(val1) == current_hash.end())// if val1 is invalid attribute name
			{
				cout << endl;
				cout <<"*************ERROR****************************" << endl;
				cout <<"         ATTRIBUTE "+val1+" NOT FOUND        "<<endl;
				cout <<"*************ERROR****************************" << endl;
				cout << endl;
				return -1;
			}


			// obtain data type of val1 and subsequently place in icol1 for integer,dcol1 for double and scol1 for string
			int type1 = current_table[current_hash[val1]][j].type;
			if (type1 == 1)// val1 is integer type
				icol1 =  current_table[current_hash[val1]][j].idata;
			if (type1 == 2)// val1 is double type
				dcol1 =  current_table[current_hash[val1]][j].ddata;
			else// val1 is string type
				scol1 =  current_table[current_hash[val1]][j].sdata;


			



			if (val2[0]=='\'')// if val2 is a string value 

			{
				scol2 = val2.substr(1,val2.length()-2);// if val2 is string value 

				if (type1!=3)//if attribute val1 is not string value
				{
					cout << endl;
					cout <<"*************TYPE VALUE MISMATCH ERROR**************" << endl;
					cout <<"           ATTRIBUTE "+val1+" NOT STRING        "<<endl;
					cout <<"*************TYPE VALUE MISMATCH ERROR**************" << endl;
					cout << endl;
					return -1;

				}

			}

			else// val2 can be name of column or number
			{
				map<string,int>::iterator it = current_hash.find(val2);

				if (it != current_hash.end())
				{
					int type2 = current_table[current_hash[val2]][j].type;

				if (type2 == 1)
					icol2 =  current_table[current_hash[val2]][j].idata;
				if (type2 == 2)
					dcol2 =  current_table[current_hash[val2]][j].ddata;
				else
					scol2 =  current_table[current_hash[val2]][j].sdata;

				if (type1!=type2)//  if domains of both values va1,val2 are different then raise error.
				{
					cout << endl;
					cout <<"*****************************TYPE VALUE MISMATCH ERROR*******************************" << endl;
					cout <<"           ATTRIBUTE "+val1+" AND ATTRIBUTE "+val2+" HAVE DIFFERENT TYPES"<<endl;
					cout <<"*****************************TYPE VALUE MISMATCH ERROR*******************************" << endl;
					cout << endl;
					return -1;

				}




				}

				else
				{
					if (val2.find(".") != string::npos)// if val2 is floating point value
					{
						dcol2 = atof(val2.c_str());

						if (type1!=2)
						{
							cout << endl;
							cout <<"*************TYPE VALUE MISMATCH ERROR**************" << endl;
							cout <<"           ATTRIBUTE "+val1+" NOT DOUBLE        "<<endl;
							cout <<"*************TYPE VALUE MISMATCH ERROR**************" << endl;
							cout << endl;
							return -1;
						}
					}


					//val2 is string value but not in quotes then raise error
					else if((val2[0]>=97 and val2[0]<=122) or (val2[0]>=65 and val2[0]<=90) )
					{

							cout << endl;
							cout <<"*************TYPO ERROR****************************" << endl;
							cout <<"           VALUE "+val2+" NOT IN quotes(' ')        "<<endl;
							cout <<"*************TYPO ERROR****************************" << endl;
							cout << endl;
							return -1;
						
					}
					else
					{

						icol2 = atoi(val2.c_str());//if val2 is integer

						if (type1!=1)
						{
							cout << endl;
							cout <<"*************TYPE VALUE MISMATCH ERROR**************" << endl;
							cout <<"           ATTRIBUTE "+val1+" NOT INTEGER        "<<endl;
							cout <<"*************TYPE VALUE MISMATCH ERROR**************" << endl;
							cout << endl;
							return -1;
						}



					}
				}


			}



			if (andor == 1)
			{
			// call compare function with approprite arguments 
			if (scol1.length() and scol2.length())
				flag = flag and compare(scol1,scol2,oper);// when val1 and val2 are strings

			else if(icol1 != -999 and icol2 != -999)
				flag = flag and compare(icol1,icol2,oper);// when val1 and val2 are integers
			else	
				flag = flag and compare(dcol1,dcol2,oper);// when val1 and val2 are double

			}

			else
			{
				// call compare function with approprite arguments 
			if (scol1.length() and scol2.length())
				flag = flag or compare(scol1,scol2,oper);// when val1 and val2 are strings

			else if(icol1 != -999 and icol2 != -999)
				flag = flag or compare(icol1,icol2,oper);// when val1 and val2 are integers
			else	
				flag = flag or compare(dcol1,dcol2,oper);// when val1 and val2 are double

			}
			
		 
		}
		
		if (flag == true)
			index_store.push_back(j);//if tuple satisfies all conditions push its index in index_store
	}

	vll secondary;//copy entire current table to secondary table
	for (int i = 0; i < current_table.size(); ++i)
	{
		vl array;
		for (int j = 0; j < index_store.size(); ++j)
		{
			array.push_back(current_table[i][index_store[j]]);
		}
		secondary.push_back(array);
		array.clear();
	}

	current_table.clear();// clear current table


	//copy back only those tuples which are in index store to the current table
	for (int i = 0; i < secondary.size(); ++i)
	{
		current_table.push_back(secondary[i]);
	}

	

	// if everything runs fine return 1.
	return 1;
	
	
}


/*************************eval function***************************************

->eval function from the given query extracts the two operands and an operator.
->val1-> operand1 value
->val2-> operand2 value
->oper-> operator value

****************************************************************************/


void eval(string query,string &val1,string &val2,string &oper)
{
	
	for(int i=0; i<query.length(); i++)
	{
		
		//if character is between a-z or A-Z it will an attribute name
	 	while ((query[i]>=97 and query[i]<=122) or (query[i] >=65 and query[i] <=90) )
	 	{
	 		val1+=query[i];
	 		i++;
	 	}
	 	
	 	// if character is not between a-z or A-Z or a number then it will operator symbol
	 	while(query[i] !='\'' and query[i]!='-' and !((query[i]>=97 and query[i]<=122) or (query[i] >=65 and query[i] <=90) or(query[i] >=48 and query[i] <=57)))
	 	{
	 		oper+=query[i];
	 		i++;
	 	}
	 

		// rest will be operand2
		while(i < query.length())
		{
			val2 += query[i];
			i++;
		}
	
	
	}
}


/***************************************compare function**********************************************

compare functions compares two values with reference to an operator.
-> val1 and val2 are objects of class value.
-> we compare the two values held by these objects using '==' operator defined in value class.
-> If the condition(operand1|operator|operand2) satisfies we return true otherwise return false.

*****************************************************************************************************/

template<typename T>
bool compare(T val1,T val2,string oper)
{
		if (oper == "=")
			return val1==val2;
		if (oper == "!=")
			return val1!=val2;
		if (oper == ">=")
			return val1>=val2;
		if (oper == "<=")
			return val1<=val2;
		if (oper == ">")
			return val1>val2;
		if (oper == "<")
			return val1<val2;
}


/***********************************************set_difference function *********************************
-> performs set difference between two sets.
-> We input two tables , which are stored in two vectors tA and tB respectively.
-> Checks if both the relations are of same arity,if not then generates error.
-> Checks if domain of the ith atrribute of table A and the ith attribute of table B must be the same,for all i. 
-> We make two sets s1 and s2 where elements of both sets will be tuples of table A and table B repectively.
-> For each element of s2 we check if that element is present in s1,if it is we erase that element from s1.
-> We restore the result in set s1 back to currently active table.

********************************************************************************************************/


int set_difference(vll &tA,vll &tB)
{
	
	if (tA.size()!=tB.size())// check if both tables are of same size.
	{
		cout << endl;
		cout <<"*************SIZE MISMATCH ERROR**************" << endl;
		cout <<"           RELATIONS HAVE DIFFERENT SIZES;        "<<endl;
		cout <<"*************SIZE MISMATCH ERROR**************" << endl;
		cout << endl;
		return -1;
	}


	for (int i = 0; i < tA.size(); ++i)
	{

		//Check if domain of the ith atrribute of table A and the ith attribute of table B must be the same,for all i. 
		if(tA[i][1].type != tB[i][1].type)

		{
			cout << endl;
			cout <<"*************COMPATIBILITY ERROR*****************************" << endl;
			cout <<"           RELATIONS ARE NOT DIFFRENCE COMPATIBLE;        "<<endl;
			cout <<"*************COMPATIBILITY ERROR*****************************" << endl;
			cout << endl;
			return -1;
		}

	}

	set<vl,cmp> s1,s2;


	int sizeA = tA.size();
	vl headings,x,y;


	// prepare set s1 by inserting tuples from tableA to s1
	for (int i = 0; i < tA[0].size(); ++i)
	{
		vl temp;
		for (int j = 0; j < tA.size(); ++j)
		{
			if (i!=0)
				temp.push_back(tA[j][i]);
			else
				headings.push_back(tA[j][i]);
		}
		if (i!=0)
			s1.insert(temp);
	}


	// prepare set s2 by inserting tuples from tableB to s2
	for (int i = 1; i < tB[0].size(); ++i)
	{
		vl temp;
		for (int j = 0; j < tB.size(); ++j)
		{
			temp.push_back(tB[j][i]);
		}
		s2.insert(temp);
	}

	set<vl,cmp>::iterator it,it1,it2;

	//For each element of s2 we check if that element is present in s1,if it is we erase that element from s1.
	for ( it1 = s2.begin(); it1!=s2.end(); ++it1)
	{

		x = *it1;
		for ( it2 = s1.begin(); it2 !=s1.end(); ++it2)
		{
			y = *it2;

			if (x==y)
				s1.erase(it2);
		}
	}

	for (int i = 0; i < tA.size(); ++i)
	{
		tA[i].clear();
	}

	for (int i = 0; i < tA.size(); ++i)
	{
		tA[i].push_back(headings[i]);
	}


	//restore the result in set s1 back to currently avtive table.
	for ( it = s1.begin(); it!=s1.end(); ++it)
	{
		vl  temp = *it;
		for (int i = 0; i < tA.size(); ++i)
		{
			tA[i].push_back(temp[i]);
		}
	}

	return 1;// if query gets successfully executed return 1.

}

/**************************************************set-union function****************************************** 
-> performs union of two sets.
-> We input two tables , which are stored in two vectors tA and tB respectively.
-> Checks if both the relations are of same arity,if not then generates error.
-> Checks if domain of the ith atrribute of table A and the ith attribute of table B must be the same,for all i. 
-> We make two sets s1 and s2 where elements of both sets will be tuples of table A and table B repectively.
-> For each element of s2 we perform insert operation and insert that element in s1.If that element(tuple) 
	was already present in s1 it will not be inserted since s1 is a set.
-> We restore the result in set s1 back to currently active table.

***************************************************************************************************************
*/


int  set_union(vll &tA,vll &tB)
{


	if (tA.size()!=tB.size())// check if both tables are of same size.
	{
		cout << endl;
		cout <<"*************SIZE MISMATCH ERROR**************" << endl;
		cout <<"           RELATIONS HAVE DIFFERENT SIZES;        "<<endl;
		cout <<"*************SIZE MISMATCH ERROR**************" << endl;
		cout << endl;
		return -1;
	}


	for (int i = 0; i < tA.size(); ++i)
	{
		
		//Check if domain of the ith atrribute of table A and the ith attribute of table B must be the same,for all i. 
		if(tA[i][1].type != tB[i][1].type)

		{
			cout << endl;
			cout <<"*************COMPATIBILITY ERROR**************" << endl;
			cout <<"           RELATIONS ARE NOT UNION COMPATIBLE;        "<<endl;
			cout <<"*************COMPATIBILITY ERROR**************" << endl;
			cout << endl;
			return -1;
		}

	}
	set<vl,cmp> s1,s2;


	int sizeA = tA.size();
	vl headings,x,y;

	// prepare set s1 by inserting tuples from tableA to s1
	for (int i = 0; i < tA[0].size(); ++i)
	{
		vl temp;
		for (int j = 0; j < tA.size(); ++j)
		{
			if (i!=0)
				temp.push_back(tA[j][i]);
			else
				headings.push_back(tA[j][i]);
		}
		if (i!=0)
			s1.insert(temp);
	}

	// prepare set s2 by inserting tuples from tableB to s2
	for (int i = 1; i < tB[0].size(); ++i)
	{
		vl temp;
		for (int j = 0; j < tB.size(); ++j)
		{
			temp.push_back(tB[j][i]);
		}
		s2.insert(temp);
	}

	
	//For each element of s2 we insert that element in s1.
	set<vl,cmp>::iterator it,it1,it2;
	for ( it1 = s2.begin(); it1!=s2.end(); ++it1)
		s1.insert(*it1);

	for (int i = 0; i < tA.size(); ++i)
	{
		tA[i].clear();
	}

	for (int i = 0; i < tA.size(); ++i)
	{
		tA[i].push_back(headings[i]);
	}

	//We restore the result in set s1 back to currently active table.
	for ( it = s1.begin(); it!=s1.end(); ++it)
	{
		vl  temp = *it;
		for (int i = 0; i < tA.size(); ++i)
		{
			tA[i].push_back(temp[i]);
		}
	}

	return 1;

}
/**********************************************cross_product*****************************************************

-> performs cross product between two tables
->This function takes two file names as input.
->Checks if both names are valid or not.
->We get two tables in crtable1 and crtable2.
->performs cross product of crtable1 and crtable2.
->updates current hash to map to include mappings of attribute to new indices of current table.
->update current rename hash to include mappings of modified attributes (attribute names prefixed with "filename.") to 
  original names.

*****************************************************************************************************************/



int cross_product(string file1,string file2)
{

	vll *current;
	map<string,int> *mp;
	map<string,string> *rnmhash;



	// check if both file names are valid or not
	if ((mytables.find(file1) == mytables.end()) or (mytables.find(file2) == mytables.end()))
	{
		cout << endl;
		cout <<"**********************NAME ERROR*********************" << endl;
		cout <<"                  ENTER  VALID RELATION NAMES       "<<endl;
		cout <<"**********************NAME ERROR*********************" << endl;
		cout << endl;
		return -1;


	}


	if (table_operates == 1)
	{  
		current = &tableA;
		mp      = &hash_tableA;
		rnmhash = &rename_hashA;
	}

	else{
		current = &tableB;
		mp      = &hash_tableB;
		rnmhash = &rename_hashB;

	}


	vll &current_table = *current;
	map<string,int> &current_hash = *mp;
	map<string,string> &curr_rnm =  *rnmhash;


	vll  &crtable1 = *mytables[file1];
	vll  &crtable2 = *mytables[file2];


	//prepare current table's attribute names from crtable1 
	for (int i = 0; i < crtable1.size(); ++i)
	{	vl temp;
		temp.push_back(crtable1[i][0]);
		current_table.push_back(temp);
	}

	//prepare current table's attribute names  from crtable2 
	for (int i = 0; i < crtable2.size(); ++i)
	{
		vl temp;
		temp.push_back(crtable2[i][0]);
		current_table.push_back(temp);
	}



	// perform cross product between crtable1 and crtable2. 
	for (int i = 1; i < crtable1[0].size(); ++i)
	{
		vl temp;
		for (int j= 0; j < crtable1.size(); ++j)
		{
			temp.push_back(crtable1[j][i]);
		}


		for (int k = 0; k < crtable2[0].size()-1; ++k)
		{
			for (int l = 0; l < temp.size(); ++l)
			{
				current_table[l].push_back(temp[l]);
			}

		}

	}

	for (int i = 0; i < crtable2.size(); ++i)
	{
		vl temp;
		for (int j = 1; j < crtable2[0].size(); ++j)
		{
			temp.push_back(crtable2[i][j]);
		}

		for (int k = 0; k < crtable1[0].size(); ++k)
		{
			for (int l = 0; l < temp.size(); ++l)
			{
				current_table[i+crtable1.size()].push_back(temp[l]);
			}
		}
	}

	//updates current hash to map to include mappings of attribute to new indices of current table.

	for (int i = 0; i < current_table.size(); ++i)
			current_hash[current_table[i][0].sdata] = i;

	//update current rename hash to include mappings of modified attributes (attribute names prefixed with "filename.") to 
  	//original names.

	for (int i = 0; i < crtable1.size(); ++i)
	{
		current_hash[file1+"."+crtable1[i][0].sdata] = i;
		curr_rnm[file1+"."+crtable1[i][0].sdata] = crtable1[i][0].sdata;
	}


	//update current rename hash to include mappings of modified attributes (attribute names prefixed with "filename.") to 
  	//original names.

	for (int i = 0; i < crtable2.size(); ++i)
	{
		current_hash[file2+"."+crtable2[i][0].sdata] = i+crtable1.size();
		curr_rnm[file2+"."+crtable2[i][0].sdata] = crtable2[i][0].sdata;
	}

	return 1;	
	
}


/********************************clear_containers********************************

->This function clears all the containers after a query gets execued succesfully.
->By clearing the containers we make the conatiners ready for the next query.

*******************************************************************************/

void clear_containers()
{

	tableA.clear();
	tableB.clear();
	hash_tableA.clear();
	hash_tableB.clear();
	rename_hashA.clear();
	rename_hashB.clear();
	mytables.clear();
	table_operates = 1;
}

/*************************************display_result***************************

This Function displays the table in vector v in correct tabular form.

******************************************************************************/

void  display_result(vll &v,string name)


{

	set<vl,cmp> s1,s2;
	set<vl,cmp>::iterator it,it1,it2;
	vl headings;

	//prepare set s1 from vector v
	for (int i = 0; i < v[0].size(); ++i)
	{
		vl temp;
		for (int j = 0; j < v.size(); ++j)
		{
			if (i!=0)
				temp.push_back(v[j][i]);
			else
				headings.push_back(v[j][i]);
		}
		if (i!=0)
			s1.insert(temp);
	}

	//restore result  in s1 back to v.


	for (int i = 0; i < v.size(); ++i)
	{
		v[i].clear();
	}

	for (int i = 0; i < v.size(); ++i)
	{
		v[i].push_back(headings[i]);
	}



	for ( it = s1.begin(); it!=s1.end(); ++it)
	{
		vl  temp = *it;
		for (int i = 0; i < v.size(); ++i)
		{
			v[i].push_back(temp[i]);
		}
	}




		if (name == "")
			name = "RESULT";

		cout << endl;
    	cout <<"\t\t\t\t\t\t*********************"<< endl;
    	cout << "\t\t\t\t\t\t  RELATION "+name<< endl;
    	cout <<"\t\t\t\t\t\t*********************"<< endl;
    	cout << endl;
    	cout << "\t\t\t\t";
    	for (int i = 0; i < v.size(); ++i)
    	{
    		
    		for (int j = 0; j < 20; ++j)
    		{
    			cout << "-";
    		}
    	}
    	cout << endl;
    	for (int i = 0; i < v[0].size(); ++i)
    	{
    		cout << "\t\t\t\t";
    		for (int j = 0; j < v.size(); ++j)
    		{

    			if (v[j][i].type == 1)
    				printf("%15d",v[j][i].idata );

    			if (v[j][i].type == 2)
    				printf("%15f",v[j][i].ddata );

    			if (v[j][i].type == 3)
    				printf("%15s",(v[j][i].sdata).c_str() );
    		}
    		if (i == 0)
    		{  
    			cout << endl;
    			cout << "\t\t\t\t";
    			for (int k = 0; k < v.size(); ++k)
    			{
    				for (int l = 0; l < 20; ++l)
    					cout << "-";
    			}

    		}
    		cout << endl;
    	}
    	cout << "\t\t\t\t";
    	for (int i = 0; i < v.size(); ++i)
    	{

    		for (int j = 0; j < 20; ++j)
    		{
    			cout << "-";
    		}
    	}
    	cout << endl;
    	cout << endl;
}


/******************************check_balanced_paranthesis********************

-> Checks if all the paranthesis in query are balanced or not.

****************************************************************************/
int check_balanced_paraenthesis(string query)
{
	stack<char> stk;
	for (int i = 0; i < query.length(); ++i)
	{
		if (query[i] == '(' or query[i] == '[' or query[i] == '{' )// push left bracket in stack
			stk.push(query[i]);
		else if (query[i] == ')' or query[i] == ']'  or query[i] == '}') // if incoming bracket is closed, compare with top elelment of stack
		{
			if (query[i] == ')')
			{
				if (stk.top() != '(')
					return -1;
			}

			else if (query[i] == ']')
			{
				if (stk.top() != '[')
					return -1;
			}

			else if  (query[i] == '}')
			{
				if (stk.top() != '{')
					return -1;
			}

			stk.pop();
		}
	}

	if (stk.empty())//if stack is empty means all parenthesis are balanced.
		return 1;
	else
		return -1;
}