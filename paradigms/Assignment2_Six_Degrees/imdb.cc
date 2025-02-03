using namespace std;
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring> 
#include "imdb.h"

const char *const imdb::kActorFileName = "actordata";
const char *const imdb::kMovieFileName = "moviedata";

imdb::imdb(const string& directory)
{
  const string actorFileName = directory + "/" + kActorFileName;
  const string movieFileName = directory + "/" + kMovieFileName;
  
  actorFile = acquireFileMap(actorFileName, actorInfo);
  movieFile = acquireFileMap(movieFileName, movieInfo);
}

bool imdb::good() const
{
  return !( (actorInfo.fd == -1) || 
	    (movieInfo.fd == -1) ); 
}

struct playerInfoStruct {
  const void *actorFile;
  const string *name;
};

int compareActors(const void *ap, const void *bp)
{
  playerInfoStruct *tmp = (playerInfoStruct*)ap;
  int offset = *((int*)bp);
  string actorName = (const char*)tmp->actorFile + offset;
  return tmp->name->compare(actorName);
}

// you should be implementing these two methods right here... 
bool imdb::getCredits(const string& player, vector<film>& films) const { 
  int size = *((int *) actorFile);
  int *offset;
  playerInfoStruct info;
  info.name = &player;
  info.actorFile = actorFile;
  offset = (int*)bsearch(&info, (int*)actorFile + 1, size, sizeof(int), compareActors);
  if (offset == NULL)return false;
  int realLength = player.length() + 1;
  if(realLength%2==1)realLength++;
  short k = *((short *) ((char *)actorFile + *offset + realLength));
  int moviesOffset = *offset + realLength + 2;
  if(moviesOffset % 4 != 0)moviesOffset+=2;
  for(int i = 0; i < k; i++){
    int currMovie = *((int *) ((char *) actorFile + moviesOffset) + i);
    string currTitle = (const char *) movieFile + currMovie;
    int movieYear = 1900 + *((char *) movieFile + currMovie + currTitle.size() + 1);
    film curr;
    curr.title = currTitle;
    curr.year = movieYear;
    films.push_back(curr);
  }
  return true; 
}

struct movieInfoStruct{
  const void *movieFile;
  const film *movie;
};

int compareMovies(const void *ap, const void *bp){
  movieInfoStruct *tmp = (movieInfoStruct *) ap;
  int offset = *((int *) bp);
  film movie;
  movie.title = (const char *) tmp->movieFile + offset;
  movie.year = 1900 + *((char *) tmp->movieFile + offset + movie.title.length() + 1);
  if(*(tmp->movie) < movie){
    return -1;
  }else if(*(tmp->movie) == movie){
    return 0;
  }else {
    return 1;
  }
}

bool imdb::getCast(const film& movie, vector<string>& players) const { 
  int size = *((int *) movieFile);
  int *offset;
  movieInfoStruct info;
  info.movieFile = movieFile; 
  info.movie = &movie;
  offset = (int*)bsearch(&info, (int*)movieFile + 1, size, sizeof(int), compareMovies);
  if (offset == NULL)return false;
  int filmLength = movie.title.length() + 2;
  if(filmLength%2==1)filmLength++;
  short k = *((short *) ((char *)movieFile + *offset + filmLength));
  int actorsOffset = *offset + filmLength + 2;
  if(actorsOffset % 4 != 0)actorsOffset+=2;
  for(int i = 0; i < k; i++){
    int currActor = *((int *) ((char *) movieFile + actorsOffset) + i);
    string actor = (const char*)actorFile + currActor;
    players.push_back(actor);
  }
  return true;
  }

imdb::~imdb()
{
  releaseFileMap(actorInfo);
  releaseFileMap(movieInfo);
}

// ignore everything below... it's all UNIXy stuff in place to make a file look like
// an array of bytes in RAM.. 
const void *imdb::acquireFileMap(const string& fileName, struct fileInfo& info)
{
  struct stat stats;
  stat(fileName.c_str(), &stats);
  info.fileSize = stats.st_size;
  info.fd = open(fileName.c_str(), O_RDONLY);
  return info.fileMap = mmap(0, info.fileSize, PROT_READ, MAP_SHARED, info.fd, 0);
}

void imdb::releaseFileMap(struct fileInfo& info)
{
  if (info.fileMap != NULL) munmap((char *) info.fileMap, info.fileSize);
  if (info.fd != -1) close(info.fd);
}
