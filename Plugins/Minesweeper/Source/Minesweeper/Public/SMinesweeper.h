#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "UnrealEd.h"
#include "SlateBasics.h"
#include "SlateExtras.h"
#include "Widgets/Input/SSpinBox.h"

class SMinesweeper : public SCompoundWidget {

public:
	SLATE_BEGIN_ARGS(SMinesweeper)
	{}

	SLATE_END_ARGS()

	void Construct(const FArguments &InArgs);

private:
	FReply on_generate_grid(); // (re)populates field_box with new buttons
	void on_cell_clicked(uint16 ind); // process cell click event

	// grid parameters
	uint8 grid_width = 20;
	uint8 grid_height = 10;
	uint16 grid_nmines = 30; // number of mines
	uint16 grid_nremaining; // statistics: how many cells uncovered so far

	// the playfield is a cellular automaton, where each cell is represented
	// by a byte:
	//	bit 7: is the mine present?
	//	bit 6: is this cell uncovered?
	//	bits 0-3: since the mines don't move, we store number of
	//			neighboring mines in the lower half of the byte.
	TArray<uint8> cell_state;
	const uint8 CELL_STATE_MINED = (1 << 7);
	const uint8 CELL_STATE_UNCOVERED = (1 << 6);

	bool is_mined(uint16 ind) {
		return cell_state[ind] & CELL_STATE_MINED;
	}

	void set_mine(uint16 ind) {
		cell_state[ind] |= CELL_STATE_MINED;
	}

	bool is_uncovered(uint16 ind) {
		return cell_state[ind] & CELL_STATE_UNCOVERED;
	}

	void set_uncovered(uint16 ind) {
		if(is_uncovered(ind)) {
			return;
		}
		cell_state[ind] |= CELL_STATE_UNCOVERED;
		if(grid_nremaining) {
			--grid_nremaining;
		}
	}

	uint8 get_neighbors(uint16 ind); // how many mines around this tile?

	void visit_cell(uint16 ind); // naive recursive graph traversal

	void reveal_mines(); // when player clicks on a mined cell

	// each button corresponds to a cell in CellState array
	TArray<TSharedPtr<SBox>> cell_wrap_box;

	// container widget for the field buttons
	TSharedPtr<SVerticalBox> grid_wrap_box;

	// container widget for the status text
	TSharedPtr<SBox> status_wrap_box;

	// field parameters input spinboxes
	TSharedPtr<SSpinBox<uint8>> grid_width_spinbox;
	TSharedPtr<SSpinBox<uint8>> grid_height_spinbox;
	TSharedPtr<SSpinBox<uint16>> grid_nmines_spinbox;
};
