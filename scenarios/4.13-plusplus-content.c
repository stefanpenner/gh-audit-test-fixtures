// Scenario 4.13: a clean revert whose content lines begin with ++ / --.
// Naive diff parsers drop lines serialized as "+++..."/"---..." as file
// headers; gh-audit's hunk-aware parser must compare them as content so
// this revert diff-verifies clean.
int main(void) {
    int x = 0;
    ++x;
    --x;
    ++x; ++x;
    --x; --x;
    return x;
}
