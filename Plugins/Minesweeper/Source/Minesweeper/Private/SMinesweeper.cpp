#include "SMinesweeper.h"

void SMinesweeper::Construct(const FArguments &InArgs) {
	ChildSlot.VAlign(VAlign_Top).Padding(5.f)
	[

		SNew(SScrollBox)
		+ SScrollBox::Slot()
		.VAlign(VAlign_Top)
		.Padding(5.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 5.f, 0.f, 0.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2, 0, 0, 0)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(
						TEXT("Width:")))
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2, 0, 0, 0)
				.VAlign(VAlign_Center)
				[
					SAssignNew(grid_width_spinbox,
							SSpinBox<uint8>)
					.Value(grid_width)
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2, 0, 0, 0)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(
						TEXT(" Height:")))
				]


				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2, 0, 0, 0)
				.VAlign(VAlign_Center)
				[
					SAssignNew(grid_height_spinbox,
							SSpinBox<uint8>)
					.Value(grid_height)
				]


				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2, 0, 0, 0)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(
						TEXT(" Number of Mines:")))
				]


				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2, 0, 0, 0)
				.VAlign(VAlign_Center)
				[
					SAssignNew(grid_nmines_spinbox,
							SSpinBox<uint16>)
					.Value(grid_nmines)
				]

			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 5.f, 0.f, 0.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2, 0, 0, 0)
				.VAlign(VAlign_Center)
				[
					SNew(SButton)
					.Text(FText::FromString(
						"Generate New Grid"))
					.OnClicked(this,
					&SMinesweeper::on_generate_grid)
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2, 0, 0, 0)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(
						TEXT(" Status:")))
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2, 0, 0, 0)
				.VAlign(VAlign_Center)
				[
					SAssignNew(status_wrap_box, SBox)
					[
						SNew(STextBlock)
						.ColorAndOpacity(
						FColor(100, 100, 255, 255))
						.Text(FText::FromString(
					TEXT(" Welcome to Minesweeper! Press "
					"'Generate' button to start the game.")))
					]

				]

			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 5.f, 0.f, 0.f)
			[
				SAssignNew(grid_wrap_box, SVerticalBox)
			]

		]
	];
}

FReply SMinesweeper::on_generate_grid() {
	// update status text
	status_wrap_box->SetContent(
		SNew(STextBlock)
		.ColorAndOpacity(
			FColor(230, 230, 50, 255))
		.Text(FText::FromString(
		TEXT(" Game started! Left click on the square buttons to play.")))
	);

	// update the grid parameters according to the values in input spinboxes
	grid_width = grid_width_spinbox->GetValue();
	grid_height = grid_height_spinbox->GetValue();

	// calculate the total number of cells
	uint16 ncells = grid_width * grid_height;

	// we can't have more mines than cells
	grid_nmines = grid_nmines_spinbox->GetValue();
	grid_nmines = grid_nmines > ncells? ncells: grid_nmines;

	// reset the remaining counter
	grid_nremaining = ncells - grid_nmines;

	// empty and resize both arrays holding the obsolete grid data
	cell_state.Empty();
	cell_state.SetNum(ncells);
	cell_wrap_box.Empty();
	cell_wrap_box.SetNum(ncells);

	// randomly distribute mines across the grid (naive approach)
	for(int16 i = 0; i < grid_nmines; ++i) {
		while(1) {
			int16 ind = FMath::RandRange(0, ncells - 1);
			if(!is_mined(ind)) {
				cell_state[ind] |= CELL_STATE_MINED;
				break; // mine placed, now exit the inner loop
			}
		}
	}

	// mines are placed, mark every cell's neighboring mine count bits
	for(int16 i = 0; i < ncells; ++i) {
		cell_state[i] |= get_neighbors(i);
	}

	// i,j matrix: i for row, j for column
	grid_wrap_box->ClearChildren();
	for(int8 i = 0; i < grid_height; ++i) {
		TSharedPtr<SHorizontalBox> row;

		grid_wrap_box->AddSlot()
		.Padding(0, 0, 2, 0)
		[
			SAssignNew(row, SHorizontalBox)
		];

		for(int8 j = 0; j < grid_width; ++j) {
			int16 ind = i * grid_width + j; // cell's array index
			row->AddSlot()
			.AutoWidth()
			[
				SAssignNew(cell_wrap_box[ind], SBox)
				.WidthOverride(20.f)
				.HeightOverride(20.f)
				[
					SNew(SButton)
					.ButtonColorAndOpacity(
						FColor(240, 240, 240, 255))
					.OnClicked_Lambda([this, ind]() {
						on_cell_clicked(ind);
						return FReply::Handled();
					})
//					.Text(FText::FromString("F"))
				]
			];
		}
	}
	return FReply::Handled();
}

uint8 SMinesweeper::get_neighbors(uint16 ind) {
	uint8 n = 0; // number of neighboring mines

	// check bounds
	bool west = (ind % grid_width),
		east = ((ind % grid_width) < (grid_width - 1)),
		north = (ind >= grid_width),
		south = ((ind / grid_width) < (grid_height - 1));

	// visit every neighbor within the bounds
	if(west) {
		n += is_mined(ind - 1);
		if(north) {
			n += is_mined(ind - grid_width - 1);
		}
		if(south) {
			n += is_mined(ind + grid_width - 1);
		}
	}
	if(east) {
		n += is_mined(ind + 1);
		if(north) {
			n += is_mined(ind - grid_width + 1);
		}
		if(south) {
			n += is_mined(ind + grid_width + 1);
		}
	}
	if(north) {
		n += is_mined(ind - grid_width);
	}
	if(south) {
		n += is_mined(ind + grid_width);
	}

	return n;
}

void SMinesweeper::visit_cell(uint16 ind) {
	// check if this cell has already been uncovered - then no need to proceed
	if(is_uncovered(ind)) {
		return;
	}

	// mark this cell as uncovered
	set_uncovered(ind);

	// mark the button accordingly
	uint8 neighbors = cell_state[ind] & 15;
	bool mined = is_mined(ind);
	FText ftext = (neighbors && !mined)?
			FText::FromString(FString::FromInt(neighbors)): FText();
	FColor fcolor = mined? FColor(255, 0, 0, 255): FColor(200, 255, 200, 255);
	cell_wrap_box[ind]->SetContent(
			SNew(SButton)
			.ButtonColorAndOpacity(fcolor)
			.Text(ftext)

	);

	// if this cell has neighbors, or is mined - stop right here
	if(neighbors || mined) {
		return;
	}

	// check bounds
	bool west = (ind % grid_width),
		east = ((ind % grid_width) < (grid_width - 1)),
		north = (ind >= grid_width),
		south = ((ind / grid_width) < (grid_height - 1));

	// visit every neighbor within the bounds
	if(west) {
		visit_cell(ind - 1);
		if(north) {
			visit_cell(ind - grid_width - 1);
		}
		if(south) {
			visit_cell(ind + grid_width - 1);
		}
	}
	if(east) {
		visit_cell(ind + 1);
		if(north) {
			visit_cell(ind - grid_width + 1);
		}
		if(south) {
			visit_cell(ind + grid_width + 1);
		}
	}
	if(north) {
		visit_cell(ind - grid_width);
	}
	if(south) {
		visit_cell(ind + grid_width);
	}

	return;
}

void SMinesweeper::reveal_mines() {
	uint16 ncells = grid_width * grid_height;
	for(uint16 i = 0; i < ncells; ++i) {
		if(is_mined(i)) {
			visit_cell(i);
		}
	}
}

void SMinesweeper::on_cell_clicked(uint16 ind) {
	if(!grid_nremaining) {
		return; // game finished
	}

	if(is_mined(ind)) {
		status_wrap_box->SetContent(
			SNew(STextBlock)
			.ColorAndOpacity(
				FColor(255, 10, 10, 255))
			.Text(FText::FromString(
			TEXT(" Mission Failed. Press 'Generate' to try again.")))
		);

		reveal_mines();
		grid_nremaining = 0; // mission failed
		return;
	}

	visit_cell(ind);
	if(!grid_nremaining) {
		status_wrap_box->SetContent(
			SNew(STextBlock)
			.ColorAndOpacity(
				FColor(100, 255, 100, 255))
			.Text(FText::FromString(
			TEXT(" Well Done! Press 'Generate' for a new game.")))
		);

	} else {
		FString fstr(TEXT(" Remaining safe squares to map: "));
		fstr.AppendInt(grid_nremaining);
		status_wrap_box->SetContent(
			SNew(STextBlock)
			.ColorAndOpacity(
				FColor(230, 230, 50, 255))
			.Text(FText::FromString(fstr))
		);

	}
}

