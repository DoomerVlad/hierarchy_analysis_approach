/*	��������� ������������� ������ ������� ������������������ ������
� ������� ������ ������� �������� (���) �� ������� ������ ������ ���.
�� ���������� ������������ ������ �������� ��������� ������������
���� �����, ����� ������ ������ ����������. � ����� �������� ������������
������� �������� �������� �������� ��� ������ ������������.

����� ������� �.�.
���� ��������� ������ 06.08.2022
������ 1.2
*/

//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Unit1.h"
#include <windows.h>
#include <stdio.h>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TWindow *Window;

bool help = false;                      // ���� ���� ������
int prev_window = 0;                    // ����������� ������ ����
int count_windows = 0;                  // ����� ����
double cells_�rit[3][3] = {0};  	    // ������� ��������� ���������
double relative_weight_crit[3] = {0};   // ������������� ��� ���������
double cells_CPU[3][3] = {0};   	    // ������ �����������
double relative_weight_CPU[3] = {0};    // ������������� ���� �����������
										// �� ������� ���������
double relative_weight_CPU_crit[3][3];  // �������� ������� �������������� ����
										// ����������� �� ������� ��������
double result_crit[3] = {0};            // �������� �������� ��������
										// ��� ������ ������������
bool unblock_cells = 0;                 // ���������� ����� � ��������
bool global_error = false;              // true - ���������� ������
AnsiString error_text;                  // ����� ������

bool first_digit[3][3] = {0};
bool first_comma[3][3] = {0};
AnsiString cells_strings[9];
int prev_cell_number = 0;


//                        �������� ����
//---------------------------------------------------------------------------
__fastcall TWindow::TWindow(TComponent* Owner)
	: TForm(Owner)
{
	// ���������� ������� ����
	clear_cells();
	window_setting();
	Table->Options<<goEditing;

	// ��������� ������ ������
	Table->Col = 2; Table->Row = 1;
}
//                        ������ "����"
//---------------------------------------------------------------------------
void __fastcall TWindow::InputClick(TObject *Sender)
{
	// ������ �������������� ���� ���������
	if(count_windows == 0)
	{
		// �������� ����������������� �����
		if(check_table())
			return;

		// ������������� ����� �������
		unblock_cells = 1;

		// ���������� �����
		for(int i = 0, limit = 1; i < 3; i++)
		{
			for(int j = 0; j < limit; j++)
				cells_�rit[i][j] = Table->Cells[i+1][j+1].ToDouble();
			limit++;
		}
		// ���������� ���������� ������� �� �������� ������
		Table->Cells[1][2] = FloatToStr(1.0 / cells_�rit[1][0]);
		Table->Cells[1][3] = FloatToStr(1.0 / cells_�rit[2][0]);
		Table->Cells[2][3] = FloatToStr(1.0 / cells_�rit[2][1]);

		// �������
		scan_table(&cells_�rit[0][0]);
		rationing(&cells_�rit[0][0]);
		calc_relative_weights(&cells_�rit[0][0], relative_weight_crit);

		// ���������� ������� ����
		count_windows = 1;
		prev_cell_number = 0;
		clear_cells();
		window_setting();
		reset_flags();

		// ��������� ������ ������
		Table->Col = 1; Table->Row = 1;

	}
	else
	{
		// ������ �������������� ���� ����������� �� ������� ��������
		if(count_windows == 1)
		{
            if(check_table())
				return;
			scan_table(&cells_CPU[0][0]);
			for(int num_crit = 0; num_crit < 3; num_crit++)
			{
				for(int i = 0; i < 3; i++)
				{
					cells_�rit[0][i] = cells_CPU[i][num_crit] /
					cells_CPU[0][num_crit];
					cells_�rit[1][i] = cells_CPU[i][num_crit] /
					cells_CPU[1][num_crit];
					cells_�rit[2][i] = cells_CPU[i][num_crit] /
					cells_CPU[2][num_crit];
				}
				rationing(&cells_�rit[0][0]);
				calc_relative_weights(&cells_�rit[0][0], relative_weight_CPU);
				relative_weight_CPU_crit[num_crit][0] = relative_weight_CPU[0];
				relative_weight_CPU_crit[num_crit][1] = relative_weight_CPU[1];
				relative_weight_CPU_crit[num_crit][2] = relative_weight_CPU[2];
			}
			// ������ �����������
			for(int i = 0; i < 3; i++)
			{
				result_crit[i] = relative_weight_crit[0] *
				relative_weight_CPU_crit[0][i] + relative_weight_crit[1] *
				relative_weight_CPU_crit[1][i] +
				relative_weight_crit[2] * relative_weight_CPU_crit[2][i];
			}

			// ������ ���� ������ �����������
			clear_cells();
			count_windows = 2;
			prev_cell_number = 0;
			window_setting();
			reset_flags();
		}
		else
		{   // ������� � ������� ����
			if(count_windows == 2)
			{
				clear_cells();
				count_windows = 0;
				prev_cell_number = 0;
				unblock_cells = 0;
				window_setting();
				reset_flags();

				// ��������� ������ ������
				Table->Col = 2; Table->Row = 1;

			}
		}
	}

}
//                        ������ "��������"
//---------------------------------------------------------------------------
void __fastcall TWindow::ClearClick(TObject *Sender)
{
	clear_cells();
}
//                          ������ "Help"
//---------------------------------------------------------------------------
void __fastcall TWindow::HelpClick(TObject *Sender)
{
	if(!help)   // ������� � ���� Help
	{
		prev_window = count_windows;
		count_windows = 3;
		window_setting();
		help = true;
	}
	else        // ������� � ����������� ����
	{
		count_windows = prev_window;
		window_setting();
		help = false;
    }

}
//                        ������� �������������� ����
//---------------------------------------------------------------------------
void calc_relative_weights(double arr[], double * relative)
{
	for(int j = 0; j < 3; j++)
		relative[j] = (arr[0 + j] + arr[3 + j] + arr[6 + j])/3;
}
//                        ���������� ������ � �������
//---------------------------------------------------------------------------
void scan_table(double arr[])
{
	for(int i = 0; i < 3; i++)
		for(int j = 0; j < 3; j++)
			arr[i*3 + j] = Window->Table->Cells[i+1][j+1].ToDouble();
}
//                       	   �������� �������
//---------------------------------------------------------------------------
void clear_cells()
{
	if(!count_windows)
	{
		// ��������������� ���������� ������� ��� ������� ����
		for(int crit = 1; crit < 4; crit++)
			Window->Table->Cells[crit][crit] = "1";
		triple_hyphen();
		Window->Table->Cells[2][1] = "";
		Window->Table->Cells[3][1] = "";
		Window->Table->Cells[3][2] = "";

	}
	else
	{   // ������ �������� ������� ��� ������� ����
        for(int i = 1; i < 4; i++)
			for(int j = 1; j < 4; j++)
				Window->Table->Cells[i][j] = "";
	}
	reset_flags();
}
//                        ������������ ��������� ������
//---------------------------------------------------------------------------
void rationing(double arr[])
{
	double sum[3] = {0};
	for(int i = 0; i < 3; i++)
	{
		sum[i] = arr[0 + i * 3] + arr[0 + i*3 + 1] + arr[0 + i*3 + 2];
		arr[0 + i*3] /= sum[i];
		arr[0 + i*3 + 1] /= sum[i];
		arr[0 + i*3 + 2] /= sum[i];
	}
}
//                        ���������� ���� � �������
//---------------------------------------------------------------------------
void window_setting(void)
{
	switch(count_windows)
	{
		// ���� ���������, ������ ����
		case 0:
		Window->Describtion->Caption = "\t������� ������������� �������� "
		"�������� 1 ��-\n���������� ��������� ���������, ������ �� ��������� "
		"����������:\n\t1 - ������ ��������;\n\t3 - ��������� �������������;"
		"\n\t5 - ������������ �������������;\n\t7 - ������������ �������������;"
		"\n\t9 - ����� ������� �������������.\n\n   ��� ������������ ����� "
		"�������� � �������� ��������� Tab.";

		// ���������� ���� "���� ���������"
		decor(1, 1, 1, 1, 0, "����", "��������", "Help");

		// ��������������� ���������� �������
		for(int crit = 1; crit < 4; crit++)
		{
			Window->Table->Cells[crit][0] = "�������� �" + FloatToStr(crit);
			Window->Table->Cells[0][crit] = "�������� �" + FloatToStr(crit);
			Window->Table->Cells[crit][crit] = "1";
		}
		triple_hyphen();
		break;

		// ���� ������ ���, ������ ����
		case 1:
		Window->Describtion->Caption = "\t������� �������������� �����������, "
		"��������:\n\n\t����� ���� - 6\n\t����� ���� (��) - 32\n\t������� (���)"
		" 3,6\n\n\t��� ������ ���������� �����\n\t����������� �������.";

		// ���������� ���� "���� ������ ���"
		decor(1, 1, 1, 1, 0, "����", "��������", "Help");

		// ���������� ������� ����� ������ ���"
		for(int CPUNum = 1; CPUNum < 4; CPUNum++)
			Window->Table->Cells[CPUNum][0] = "��� �" + FloatToStr(CPUNum);
		Window->Table->Cells[0][1] = "����� ����";
		Window->Table->Cells[0][2] = "����� ����, ��";
		Window->Table->Cells[0][3] = "�������, ���";
		break;

		// ����� ������, ������ ����
		case 2:
		Window->Describtion->Caption = "\t�������� �������� �������� ��� ������"
		" ������-\n������.\n\n";

		// ���������� ���� "����� ���������� ��������"
		decor(0, 1, 0, 1, 0, "����� ������", "", "Help");

		for(int i = 0; i < 3; i++)
			Window->Describtion->Caption += "\t\t\tS" + FloatToStr(i+1) + " = "
			+ FloatToStrF(result_crit[i], ffFixed, 10, 4) + "\n";
		Window->Input->Caption = "����� ������";
		break;

		// ���� Help, �������� ����
		case 3:
		Window->Describtion->Caption = "\t������ ��������� ������������� "
		"������������ ���������� ��� ������� ������������������ ������ "
		"\"������� ������� ��������\". ����� ���������� ��� ���-\n����� "
		"������� � �������� ������������� (��� ������-\n������ ��������). "
		"������ ������ - �������� ��������� ��������� �� ��������.";

		// ���������� ���� "Help"
		decor(0, 0, 0, 1, 1, "", "", "�����");
		break;

		default:;
		exit(1);
	}
}
//                           ���������� ����
//---------------------------------------------------------------------------
void decor(int visible_table, int visible_input_button, int visible_clear_button,
int visible_help_button, int visible_image,  const char * input_button,
const char * clear_button, const char * help_button)
{
	Window->Table->Visible = visible_table;
	Window->Input->Visible = visible_input_button;
	Window->Clear->Visible = visible_clear_button;
	Window->Help->Visible = visible_help_button;
	Window->Image->Visible = visible_image;
	Window->Input->Caption = input_button;
	Window->Clear->Caption = clear_button;
	Window->Help->Caption = help_button;
}
//                        ������� ����� � �������
//---------------------------------------------------------------------------
void triple_hyphen(void)
{
	Window->Table->Cells[1][2] = "---";
	Window->Table->Cells[1][3] = "---";
	Window->Table->Cells[2][3] = "---";
}
//                           ������ �� �������
//---------------------------------------------------------------------------
int check_table(void)
{
	error_text = "������!\n";
	if(count_windows == 0)
	{
		for(int i = 0, limit = 1; i < 3; i++)
		{
			for(int j = 0; j < limit; j++)
			{
				if(check_cells(i, j))
					return 1;
			}
			limit++;
		}
	}
	else
	{
		for(int i = 0; i < 3; i++)
			for(int j = 0; j < 3; j++)
			{
				if(check_cells(i, j))
					return 1;
			}
	}
	return 0;
}
//                  �������� ��������� ����������������� �����
//---------------------------------------------------------------------------
int check_cells(int i, int j)
{
	bool first_sign = false;    	// false - �� �������� ������ ������������
									// ������
	bool space_after_sign = false;  // false - �� �������� ���������� ������
									// ����� ������� ������������� �������
	bool space_error = false;       // true - ����������� ������ ����� ���������
	bool prev_digit = false;        // false - ���������� ������ �� �����
	bool comma_exist = false;       // true - ������� ������ �������
	bool many_comma_error = false;  // true - ������� ��������� �������
	bool only_nulls = true ;        // true - ��� ������� �������� ������
	bool not_digit_or_comma = false;// true - ����������� �� ����� � �� �������

	AnsiString input;               // ���� � ����� ������
	input = Window->Table->Cells[i+1][j+1];

	for(int ctr = 1; ctr <= input.Length(); ctr++)
	{
		//  �������� �� ������� ������� ����������� �������
		if(input[ctr] != ' ' && !first_sign)
		{
			first_sign = true;
		}

		//  �������� �� ������� ���������� �������� ����� ������������
		if(first_sign && input[ctr] == ' ' && !space_after_sign)
		{
			space_after_sign = true;
		}

		//  �������� �� ������� ���������� �������� ����� ����� �������������
		if(first_sign && space_after_sign && input[ctr] != ' ' && !space_error)
		{
            space_error = true;
			global_error = true;
			error_text += "������� ������ ������ ����� ��������� � ������� "
			+ IntToStr(i + 2) + ", ������ " + IntToStr(j + 2) + ".\n";
		}

		// �������� ������� ������ �������
		if(input[ctr] == ',' && comma_exist && !many_comma_error)
		{
			many_comma_error = true;
			global_error = true;
			error_text += "���������� ��������� ������� � ������� "
			+ IntToStr(i + 2) + ", ������ " + IntToStr(j + 2) + ".\n";
		}

		// �������� ������� ���� �� �������
		if(input[ctr] == ',' && !prev_digit && !comma_exist)
		{
			global_error = true;
			error_text += "����� ������� ���� ����� � ������� "
			+ IntToStr(i + 2) + ", ������ " + IntToStr(j + 2) + ".\n";
		}

		// ���� ������ �� ����� � �� �������
		if(!isdigit(input[ctr]) && input[ctr] != ',' && input[ctr] != ' '
		&& !not_digit_or_comma)
		{
			not_digit_or_comma = true;
			global_error = true;
			error_text += "������� ���������� ������� � ������� "
			+ IntToStr(i + 2) + ", ������ " + IntToStr(j + 2) + ".\n";
		}

		// �������� ������� � ����� ������������� �����
		if(input[ctr] != ' ' && input[ctr] != '0' && input[ctr] != ',')
		{
			only_nulls = false;
		}

		// �������� �� ������� ������� �������-�����
		if(isdigit(input[ctr]))
		{
			prev_digit = true;
		}

		// �������� ������� ������ �������
		if(input[ctr] == ',' && !comma_exist)
		{
			comma_exist = true;
		}
	}

	//  ��������� ������ ���������� �������
	if(!first_sign)
	{
		global_error = true;
		error_text += "������ ����� � ������� "
		+ IntToStr(i + 2) + ", ������ " + IntToStr(j + 2) + ".\n";
	}
	else
	{   // ���� ����� ����
		if(only_nulls)
		{
			global_error = true;
			error_text += "����� �� ������ ��������� ���� � ������� "
			+ IntToStr(i + 2) + ", ������ " + IntToStr(j + 2) + ".\n";
        }
    }

	// ����� ��������� �� ������ ����� �������� ��������� ������
	if(i == 2 && j == 2 && global_error)
	{
		ShowMessage(error_text);
		error_text.Delete(1, error_text.Length());
		global_error = 0;
		return 1;
	}
	return 0;
}
//                  ���������� ����� � �������� ������� ����
//---------------------------------------------------------------------------
void __fastcall TWindow::TableSelectCell(TObject *Sender, int ACol, int ARow,
bool &CanSelect)
{
	if(!unblock_cells)
	{
		if((ACol == 1 && ARow == 2) || (ACol == 1 && ARow == 3) ||
		(ACol == 2 && ARow == 3) || (ACol == 1 && ARow == 1) ||
		(ACol == 2 && ARow == 2) || (ACol == 3 && ARow == 3))
			CanSelect = false;
	}
	else
	{
		if((ACol == 1 && ARow == 2) || (ACol == 1 && ARow == 3) ||
		(ACol == 2 && ARow == 3) || (ACol == 1 && ARow == 1) ||
		(ACol == 2 && ARow == 2) || (ACol == 3 && ARow == 3))
			CanSelect = true;
	}
}
//                        ������ ��������� �������
//---------------------------------------------------------------------------
void __fastcall TWindow::TableKeyPress(TObject *Sender, System::WideChar &Key)
{
	WideChar comma = ',';
	int row = Table->Row - 1;
	int col = Table->Col - 1;

	// �������� �������� �������
	if(Key == '\b' && cells_strings[row*3 + col]
	[cells_strings[row*3 + col].Length()] == comma)
	{
		first_comma[row][col] = false;
	}
	else
	{   // �������� �������� ���������� ������� ������
		if(Key == '\b' && cells_strings[row*3 + col].Length() == 1)
		{
			first_digit[row][col] = false;
		}
	}

	// �������� ������� ������
	if(Key == '\b')
	{
		cells_strings[row*3 + col].Delete(cells_strings[row*3 + col].Length(), 1);
        return;
	}

	// ������ ����� ������� �� ����� �����
	if(!first_digit[row][col] && Key == comma)
	{
		Key = NULL;
	}

	// ������ ����� ������ �������
	if(first_comma[row][col] && Key == comma)
	{
		Key = NULL;
	}

	// ������ ����� �� ����� � �� �������
	if(!isdigit(Key) && Key != comma)
	{
		Key = NULL;
	}

	// ���� ������ �������
	if(first_digit[row][col] && Key == comma &&
	!first_comma[row][col])
	{
		first_comma[row][col] = true;
	}

	// ���� ������ �����
	if(isdigit(Key) && !first_digit[row][col])
	{
		first_digit[row][col] = true;
	}

	// ����������� ���������� ������
	prev_cell_number = Table->Col * 10 + Table->Row;

	// ����������� ����������� �������
	if(Key != '\b' && Key != NULL)
	{
		cells_strings[row*3 + col] += Key;
	}
	return;
}
//     ����� cells_strings, ������ ������� ������� � ���� � ������� Table
//---------------------------------------------------------------------------
void reset_flags()
{
	for(int i = 0; i < 3; i++)
		for(int j = 0; j < 3; j++)
		{
			first_digit[i][j] = false;
			first_comma[i][j] = false;
			cells_strings[3*i + j].Delete(1, cells_strings[3*i + j].Length());
		}
	return;
}
//     ����� cells_strings, ������ ������� ������� � ���� � ������� Table
//---------------------------------------------------------------------------