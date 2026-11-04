#include "Chess.h"
#include "UCI.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <cstdint>

using namespace std;

static char pieceToChar(Piece pc){char ch='.';switch(pc.type){case PieceType::PAWN:ch='P';break;case PieceType::KNIGHT:ch='N';break;case PieceType::BISHOP:ch='B';break;case PieceType::ROOK:ch='R';break;case PieceType::QUEEN:ch='Q';break;case PieceType::KING:ch='K';break;default:break;}if(pc.color==Color::BLACK)ch=(char)tolower((unsigned char)ch);return ch;}
static void printBoard(const Position&p){cout<<"\n    a b c d e f g h\n   -----------------\n";for(int r=0;r<8;r++){cout<<8-r<<" | ";for(int c=0;c<8;c++)cout<<pieceToChar(p.board[r][c])<<' ';cout<<"| "<<8-r<<'\n';}cout<<"   -----------------\n    a b c d e f g h\n\n";}
static bool parseInputMove(const string&s,Move&out){if(s.size()!=4&&s.size()!=5)return false;string x=s;for(char&ch:x)ch=(char)tolower((unsigned char)ch);if(x[0]<'a'||x[0]>'h'||x[2]<'a'||x[2]>'h'||x[1]<'1'||x[1]>'8'||x[3]<'1'||x[3]>'8')return false;out.fromRow='8'-x[1];out.fromCol=x[0]-'a';out.toRow='8'-x[3];out.toCol=x[2]-'a';out.promotion=PieceType::NONE;if(x.size()==5){switch(x[4]){case'q':out.promotion=PieceType::QUEEN;break;case'r':out.promotion=PieceType::ROOK;break;case'b':out.promotion=PieceType::BISHOP;break;case'n':out.promotion=PieceType::KNIGHT;break;default:return false;}}return true;}
static bool sameMove(const Move&a,const Move&b){return a.fromRow==b.fromRow&&a.fromCol==b.fromCol&&a.toRow==b.toRow&&a.toCol==b.toCol&&a.promotion==b.promotion;}

static void printHelp(){cout<<"Commands:\n  e2e4       make a move\n  moves      list legal moves\n  fen        show current FEN\n  setfen FEN load a position\n  depth N    set engine max depth (default 4)\n  time N     set engine time in milliseconds (default 1000)\n  perft N    run move-generation test\n  undo       undo your last move\n  new        new game\n  help       show commands\n  quit       exit\n";}

struct HistoryEntry{Move move;StateInfo state;};

int main(int argc, char** argv){
    if(argc > 1 && std::string(argv[1]) == "--uci") { runUCI(); return 0; }
    Position p=startPosition();vector<HistoryEntry> history;unordered_map<uint64_t,int> repetitions;repetitions[positionKey(p)]=1;int depth=4,timeMs=1000;
    cout<<"========================================\n       MY CHESS ENGINE - TERMINAL\n========================================\n";cout<<"Basic rules + search engine\nType 'help' for commands.\n";
    while(true){
        printBoard(p);auto legal=generateLegalMoves(p);bool check=isInCheck(p,p.sideToMove);if(legal.empty()){if(check)cout<<"Checkmate! "<<colorName(p.sideToMove==Color::WHITE?Color::BLACK:Color::WHITE)<<" wins.\n";else cout<<"Stalemate! Draw.\n";break;}if(check)cout<<"CHECK!\n";
        if(p.halfmoveClock>=100){cout<<"50-move rule draw.\n";break;}if(repetitions[positionKey(p)]>=3){cout<<"Threefold repetition detected. Draw.\n";break;}
        cout<<colorName(p.sideToMove)<<" to move > ";string input;if(!getline(cin,input))break;stringstream ss(input);string cmd;ss>>cmd;if(cmd.empty())continue;string low=cmd;for(char&c:low)c=(char)tolower((unsigned char)c);
        if(low=="quit"||low=="exit")break;
        if(low=="help"){printHelp();continue;}
        if(low=="new"){p=startPosition();history.clear();repetitions.clear();repetitions[positionKey(p)]=1;continue;}
        if(low=="undo"){if(history.empty()){cout<<"Nothing to undo.\n";continue;}auto h=history.back();history.pop_back();auto k=positionKey(p);if(auto it=repetitions.find(k);it!=repetitions.end()){if(--it->second==0)repetitions.erase(it);}undoMove(p,h.move,h.state);continue;}
        if(low=="fen"){cout<<positionToFEN(p)<<"\n";continue;}
        if(low=="moves"){for(const auto&m:legal)cout<<moveToString(m)<<' ';cout<<"\n";continue;}
        if(low=="depth"){int n;if(ss>>n&&n>=1&&n<=10){depth=n;cout<<"Depth set to "<<depth<<".\n";}else cout<<"Use depth 1..10.\n";continue;}
        if(low=="time"){int n;if(ss>>n&&n>=50&&n<=60000){timeMs=n;cout<<"Time set to "<<timeMs<<" ms.\n";}else cout<<"Use time 50..60000 ms.\n";continue;}
        if(low=="perft"){int d;if(ss>>d&&d>=1&&d<=6){Position t=p;cout<<"Perft("<<d<<") = "<<perft(t,d)<<"\n";}else cout<<"Use perft 1..6.\n";continue;}
        if(low=="divide"){int d;if(ss>>d&&d>=1&&d<=5){Position t=p;auto root=generateLegalMoves(t);std::uint64_t total=0;for(const auto&m:root){StateInfo st{};makeMove(t,m,st);auto n=perft(t,d-1);undoMove(t,m,st);cout<<moveToString(m)<<": "<<n<<"\n";total+=n;}cout<<"Total: "<<total<<"\n";}else cout<<"Use divide 1..5.\n";continue;}
        if(low=="eval"){cout<<"Evaluation: "<<evaluate(p)<<" centipawns from side-to-move perspective.\n";continue;}
        if(low=="bench"){auto t0=std::chrono::steady_clock::now();SearchResult r=searchBestMove(p,depth,timeMs);auto t1=std::chrono::steady_clock::now();auto ms=std::chrono::duration_cast<std::chrono::milliseconds>(t1-t0).count();double nps=ms? (double)r.nodes*1000.0/ms:0.0;cout<<"Bench: move="<<moveToString(r.move)<<" depth="<<r.depth<<" nodes="<<r.nodes<<" time="<<ms<<" ms nps="<<(std::uint64_t)nps<<"\n";continue;}
        if(low=="uci"){runUCI();break;}
        if(low=="setfen"){string fen,part;getline(ss,part);if(part.size()&&part[0]==' ')part.erase(0,1);Position q;if(positionFromFEN(part,q)){p=q;history.clear();repetitions.clear();repetitions[positionKey(p)]=1;cout<<"Position loaded.\n";}else cout<<"Invalid FEN.\n";continue;}
        Move user; if(!parseInputMove(cmd,user)){cout<<"Invalid command or move. Example: e2e4\n";continue;}auto it=find_if(legal.begin(),legal.end(),[&](const Move&m){return sameMove(m,user);});if(it==legal.end()){cout<<"Illegal move: "<<cmd<<"\n";continue;}
        StateInfo st{};makeMove(p,*it,st);history.push_back({*it,st});repetitions[positionKey(p)]++;
        // Engine responds as Black only. If user has switched to black with setfen, it still responds after the user move logic.
        legal=generateLegalMoves(p);if(legal.empty())continue;
        cout<<"Engine thinking... depth="<<depth<<" time="<<timeMs<<"ms\n";SearchResult result=searchBestMove(p,depth,timeMs);if(result.depth==0){result.move=legal.front();}
        cout<<"Engine plays: "<<moveToString(result.move)<<"  score="<<result.score<<" depth="<<result.depth<<" nodes="<<result.nodes<<"\n";StateInfo est{};makeMove(p,result.move,est);history.push_back({result.move,est});repetitions[positionKey(p)]++;
    }
    cout<<"Game ended.\n";return 0;
}
