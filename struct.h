

enum AnimalType
{
    MAMMAL,
    BIRD,
    REPTILE,
    AMPHIBIAN,
    FISH
};

// Beni gormeyecen

struct Species
{
    char name[50];
    char habitat[100];
};


/* Deneme sil beni */
struct Animal //kimse yok burda
{
    char name[50];
    int age; //lan okuma
    float weight;
    struct Species *species;
    enum AnimalType type;
};