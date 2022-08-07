

{
    Database *db = new Database();
    db->load();

    char board[] = {1, 2, 2, 0};

    unsigned char *entry = db->get(3, board);

    if (entry == 0) {
        cout << "No entry" << endl;
        return 0;
    }

    uint64_t snum = DB_GET_SHAPE(entry);
    vector<int> shape = numberToShapeVector(snum);

    cout << "Shape is " << snum << " " << shape << endl;
    cout << "Outcome is " << DB_GET_OUTCOME(entry) << endl;

    delete db;
}
