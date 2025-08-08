#include <mysql.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <iomanip> // Para setw()

using namespace std;

MYSQL* conn;

bool ConnectToDatabase();
void ShowMenu();
void ExecuteQuery(const string& query);
void DisplayResults();
void ManageTable(const string& table);
string GetPrimaryKeyName(const string& table);

int main()
{
	if (!ConnectToDatabase())
		return 1;

	int choice;

	while (true)
	{
		system("cls");
		ShowMenu();
		cin >> choice;
		cin.ignore();

		system("cls");

		switch (choice)
		{
		case 1: ManageTable("autores"); break;
		case 2: ManageTable("categorias"); break;
		case 3: ManageTable("libros"); break;
		case 4: ManageTable("usuarios"); break;
		case 5: ManageTable("prestamos"); break;
		case 0: mysql_close(conn); return 0;
		default: cout << "Invalid option\n"; break;
		}
	}

	return 0;
}

bool ConnectToDatabase()
{
	conn = mysql_init(NULL);

	if (!conn)
	{
		cerr << "mysql_init() failed\n";
		return false;
	}

	if (!mysql_real_connect(conn, "127.0.0.1", "root", "", "biblioteca", 3306, NULL, 0))
	{
		cerr << "Connection failed: " << mysql_error(conn) << endl;
		return false;
	}

	return true;
}

void ShowMenu()
{
	cout << "\n=== Biblioteca Manager ===\n";
	cout << "1. Manage Authors\n";
	cout << "2. Manage Categories\n";
	cout << "3. Manage Books\n";
	cout << "4. Manage Users\n";
	cout << "5. Manage Loans\n";
	cout << "0. Exit\n";
	cout << "Select an option: ";
}

void ExecuteQuery(const string& query)
{
	if (mysql_query(conn, query.c_str()) != 0)
	{
		cerr << "Query failed: " << mysql_error(conn) << endl;
	}
	else
	{
		cout << "Query executed successfully.\n";
	}
}

void DisplayResults()
{
	const int colWidth = 20;

	MYSQL_RES* res = mysql_store_result(conn);
	if (!res)
	{
		cerr << "No results or error: " << mysql_error(conn) << endl;
		return;
	}

	int numFields = mysql_num_fields(res);
	MYSQL_FIELD* fields = mysql_fetch_fields(res);
	MYSQL_ROW row;

	// Imprimir encabezados
	for (int i = 0; i < numFields; i++)
	{
		cout << left << setw(colWidth) << fields[i].name;
	}
	cout << "\n";

	// Separadores
	for (int i = 0; i < numFields; i++)
	{
		cout << string(colWidth, '-');
	}
	cout << "\n";

	// Filas
	while ((row = mysql_fetch_row(res)))
	{
		for (int i = 0; i < numFields; i++)
		{
			string value = row[i] ? row[i] : "NULL";
			cout << left << setw(colWidth) << value;
		}
		cout << "\n";
	}

	mysql_free_result(res);
}

void ManageTable(const string& table)
{
	int option;

	do
	{
		cout << "\nManaging table: " << table << endl;
		cout << "1. Insert\n";
		cout << "2. Delete\n";
		cout << "3. Update\n";
		cout << "4. View\n";
		cout << "0. Back\n";
		cout << "Select an option: ";
		cin >> option;
		cin.ignore();

		string query, input;
		int id;

		switch (option)
		{
		case 1:
		{
			if (table == "autores")
			{
				string nombre, apellido;
				cout << "Name: "; getline(cin, nombre);
				cout << "Surname: "; getline(cin, apellido);
				query = "INSERT INTO autores (nombre, apellido) VALUES ('" + nombre + "', '" + apellido + "')";
			}
			else if (table == "categorias")
			{
				string nombre;
				cout << "Category name: "; getline(cin, nombre);
				query = "INSERT INTO categorias (nombre) VALUES ('" + nombre + "')";
			}
			else if (table == "libros")
			{
				string titulo;
				int id_autor, id_categoria, anio;
				cout << "Title: "; getline(cin, titulo);
				cout << "Author ID: "; cin >> id_autor;
				cout << "Category ID: "; cin >> id_categoria;
				cout << "Publication Year: "; cin >> anio; cin.ignore();
				query = "INSERT INTO libros (titulo, id_autor, id_categoria, anio_publicacion) VALUES ('" +
					titulo + "', " + to_string(id_autor) + ", " + to_string(id_categoria) + ", " + to_string(anio) + ")";
			}
			else if (table == "usuarios")
			{
				string nombre, email;
				cout << "Name: "; getline(cin, nombre);
				cout << "Email: "; getline(cin, email);
				query = "INSERT INTO usuarios (nombre, email) VALUES ('" + nombre + "', '" + email + "')";
			}
			else if (table == "prestamos")
			{
				int id_libro, id_usuario;
				string fecha_prestamo, fecha_devolucion;
				cout << "Book ID: "; cin >> id_libro;
				cout << "User ID: "; cin >> id_usuario;
				cout << "Loan Date (YYYY-MM-DD): "; cin >> fecha_prestamo;
				cout << "Return Date (YYYY-MM-DD or NULL): "; cin >> fecha_devolucion;
				cin.ignore();
				query = "INSERT INTO prestamos (id_libro, id_usuario, fecha_prestamo, fecha_devolucion) VALUES (" +
					to_string(id_libro) + ", " + to_string(id_usuario) + ", '" + fecha_prestamo + "', " +
					(fecha_devolucion == "NULL" ? "NULL" : "'" + fecha_devolucion + "'") + ")";
			}
			ExecuteQuery(query);
			break;
		}

		case 2:
		{
			cout << "ID to delete: ";
			cin >> id;
			cin.ignore();
			string primaryKey = GetPrimaryKeyName(table);
			query = "DELETE FROM " + table + " WHERE " + primaryKey + " = " + to_string(id);
			ExecuteQuery(query);
			break;
		}

		case 3:
		{
			query = "SELECT * FROM " + table;
			if (mysql_query(conn, query.c_str()) == 0)
			{
				DisplayResults();
			}
			else
			{
				cerr << "Failed to retrieve data: " << mysql_error(conn) << endl;
				break;
			}

			string primaryKey = GetPrimaryKeyName(table);

			int idToUpdate;
			cout << "\nEnter the ID of the row you want to update: ";
			cin >> idToUpdate;
			cin.ignore();

			string column;
			cout << "Enter the column name to modify: ";
			getline(cin, column);

			string newValue;
			cout << "Enter the new value: ";
			getline(cin, newValue);

			bool useQuotes = !(newValue == "NULL" || all_of(newValue.begin(), newValue.end(), ::isdigit));
			if (useQuotes)
				newValue = "'" + newValue + "'";

			query = "UPDATE " + table + " SET " + column + " = " + newValue +
				" WHERE " + primaryKey + " = " + to_string(idToUpdate);
			ExecuteQuery(query);
			break;
		}

		case 4:
		{
			query = "SELECT * FROM " + table;
			if (mysql_query(conn, query.c_str()) == 0)
			{
				DisplayResults();
			}
			else
			{
				cerr << "Failed to retrieve data: " << mysql_error(conn) << endl;
			}
			break;
		}
		}

	} while (option != 0);
}

string GetPrimaryKeyName(const string& table)
{
	string query = "SELECT * FROM " + table + " LIMIT 1";
	if (mysql_query(conn, query.c_str()) != 0)
	{
		cerr << "Query failed: " << mysql_error(conn) << endl;
		return "";
	}

	MYSQL_RES* res = mysql_store_result(conn);
	if (!res)
	{
		cerr << "Failed to get result set: " << mysql_error(conn) << endl;
		return "";
	}

	MYSQL_FIELD* fields = mysql_fetch_fields(res);
	string primaryKeyName = fields[0].name;

	mysql_free_result(res);
	return primaryKeyName;
}
