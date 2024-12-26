#ifndef VISUALMODEHANDLER_H
#define VISUALMODEHANDLER_H

enum class VisualType {
    Character,
    Line,
    Block
};

class VisualModeHandler {
public:
    VisualModeHandler();

    void startSelection(int startY, int startX, VisualType type);
    void updateSelection(int endY, int endX);
    void clearSelection();
    bool hasSelection() const;

    void getSelectionBounds(int &startY, int &startX, int &endY, int &endX) const;
    VisualType getVisualType() const;

private:
    int selStartY, selStartX;
    int selEndY, selEndX;
    bool selectionActive;
    VisualType selType;
};

#endif
