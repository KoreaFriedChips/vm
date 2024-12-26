#include "VisualModeHandler.h"
#include <algorithm>

VisualModeHandler::VisualModeHandler() : selStartY(0), selStartX(0), selEndY(0), selEndX(0), selectionActive(false), selType(VisualType::Character) {}

void VisualModeHandler::startSelection(int startY, int startX, VisualType type) {
    selStartY = startY;
    selStartX = startX;
    selEndY = startY;
    selEndX = startX;
    selectionActive = true;
    selType = type;
}

void VisualModeHandler::updateSelection(int endY, int endX) {
    if (!selectionActive) return;
    selEndY = endY;
    selEndX = endX;
}

void VisualModeHandler::clearSelection() {
    selectionActive = false;
}

bool VisualModeHandler::hasSelection() const {
    return selectionActive;
}

void VisualModeHandler::getSelectionBounds(int &startY, int &startX, int &endY, int &endX) const {
    if (!selectionActive) {
        startY = startX = endY = endX = 0;
        return;
    }
    if (selStartY < selEndY || (selStartY == selEndY && selStartX <= selEndX)) {
        startY = selStartY; startX = selStartX;
        endY = selEndY; endX = selEndX;
    } else {
        startY = selEndY; startX = selEndX;
        endY = selStartY; endX = selStartX;
    }
}

VisualType VisualModeHandler::getVisualType() const {
    return selType;
}
