static piecedef piece1_1 = {
    {1,1,1,0},
    {0,1,0,0},
    {0,0,0,0},
    {0,0,0,0}
};

static piecedef piece1_2 = {
    {1,0,0,0},
    {1,1,0,0},
    {1,0,0,0},
    {0,0,0,0}
};

static piecedef piece1_3 = {
    {0,1,0,0},
    {1,1,1,0},
    {0,0,0,0},
    {0,0,0,0}
};

static piecedef piece1_4 = {
    {0,1,0,0},
    {1,1,0,0},
    {0,1,0,0},
    {0,0,0,0}
};

static piecedef piece2_1 = {
    {1,1,1,0},
    {1,0,0,0},
    {0,0,0,0},
    {0,0,0,0}
};
static piecedef piece2_2 = {
    {1,0,0,0},
    {1,0,0,0},
    {1,1,0,0},
    {0,0,0,0}
};
static piecedef piece2_3 = {
    {0,0,1,0},
    {1,1,1,0},
    {0,0,0,0},
    {0,0,0,0}
};
static piecedef piece2_4 = {
    {1,1,0,0},
    {0,1,0,0},
    {0,1,0,0},
    {0,0,0,0}
};

static piecedef piece3_1 = {
    {0,0,1,0},
    {1,1,1,0},
    {0,0,0,0},
    {0,0,0,0}
};
static piecedef piece3_2 = {
    {1,1,0,0},
    {0,1,0,0},
    {0,1,0,0},
    {0,0,0,0}
};
static piecedef piece3_3 = {
    {1,1,1,0},
    {1,0,0,0},
    {0,0,0,0},
    {0,0,0,0}
};
static piecedef piece3_4 = {
    {1,0,0,0},
    {1,0,0,0},
    {1,1,0,0},
    {0,0,0,0}
};

static piecedef piece4_1 = {
    {1,1,0,0},
    {0,1,1,0},
    {0,0,0,0},
    {0,0,0,0}
};
static piecedef piece4_2 = {
    {0,1,0,0},
    {1,1,0,0},
    {1,0,0,0},
    {0,0,0,0}
};
static piecedef piece4_3 = {
    {1,1,0,0},
    {0,1,1,0},
    {0,0,0,0},
    {0,0,0,0}
};
static piecedef piece4_4 = {
    {0,1,0,0},
    {1,1,0,0},
    {1,0,0,0},
    {0,0,0,0}
};

static piecedef piece5_1 = {
    {0,1,1,0},
    {1,1,0,0},
    {0,0,0,0},
    {0,0,0,0}
};

static piecedef piece5_2 = {
    {1,0,0,0},
    {1,1,0,0},
    {0,1,0,0},
    {0,0,0,0}
};
static piecedef piece5_3 = {
    {0,1,1,0},
    {1,1,0,0},
    {0,0,0,0},
    {0,0,0,0}
};
static piecedef piece5_4 = {
    {1,0,0,0},
    {1,1,0,0},
    {0,1,0,0},
    {0,0,0,0}
};

static piecedef piece6 = {
    {1,1,0,0},
    {1,1,0,0},
    {0,0,0,0},
    {0,0,0,0}
};

static piecedef piece7_1 = {
    {1,1,1,1},
    {0,0,0,0},
    {0,0,0,0},
    {0,0,0,0}
};
static piecedef piece7_2 = {
    {0,1,0,0},
    {0,1,0,0},
    {0,1,0,0},
    {0,1,0,0}
};


static struct piece allpieces[] = {
    {
        3, 0, 1, 0,
        {
            &piece1_1,
            &piece1_2,
            &piece1_3,
            &piece1_4
        }
    },
    {
        3, 0, 0, 1,
        {
            &piece2_1,
            &piece2_2,
            &piece2_3,
            &piece2_4
        },
    },
    {
        3, 1, 0, 2,
        {
            &piece3_1,
            &piece3_2,
            &piece3_3,
            &piece3_4
        }
    },
    {
        3, 0, 1, 3,
        {
            &piece4_1,
            &piece4_2,
            &piece4_3,
            &piece4_4
        }
    },
    {
        3, 0, 1, 4,
        {
            &piece5_1,
            &piece5_2,
            &piece5_3,
            &piece5_4
        }
    },
    {
        2, 1, 1, 5,
        {
            &piece6,
            &piece6,
            &piece6,
            &piece6
        }
    },
    {
        4, 0, 1, 6,
        {
            &piece7_1,
            &piece7_2,
            &piece7_1,
            &piece7_2
        }
    },


};
