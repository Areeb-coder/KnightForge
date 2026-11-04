#include "Chess.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cctype>
#include <climits>
#include <random>
#include <sstream>
#include <unordered_map>

using Clock = std::chrono::steady_clock;

namespace {
constexpr int INF = 100000000;
constexpr int MATE = 1000000;
constexpr int PIECE_VALUES[] = {0,100,320,330,500,900,0};

enum TTFlag : std::uint8_t { TT_EXACT, TT_LOWER, TT_UPPER };
struct TTEntry { int depth; int score; TTFlag flag; Move best; };
std::unordered_map<std::uint64_t, TTEntry> tt;
std::uint64_t nodes = 0;
Clock::time_point deadline;
bool stopSearch = false;

char pieceChar(Piece pc) {
    char ch='.';
    switch(pc.type){case PieceType::PAWN:ch='P';break;case PieceType::KNIGHT:ch='N';break;case PieceType::BISHOP:ch='B';break;case PieceType::ROOK:ch='R';break;case PieceType::QUEEN:ch='Q';break;case PieceType::KING:ch='K';break;default:break;}
    if(pc.color==Color::BLACK) ch=static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    return ch;
}

Color opposite(Color c){ return c==Color::WHITE?Color::BLACK:Color::WHITE; }

bool sameMove(const Move&a,const Move&b){return a.fromRow==b.fromRow&&a.fromCol==b.fromCol&&a.toRow==b.toRow&&a.toCol==b.toCol&&a.promotion==b.promotion;}

std::uint64_t fnvMix(std::uint64_t h, std::uint64_t x){ h ^= x; h *= 1099511628211ULL; return h; }

std::uint64_t hashPosition(const Position& p){
    std::uint64_t h=1469598103934665603ULL;
    for(int r=0;r<8;r++)for(int c=0;c<8;c++){
        h=fnvMix(h, static_cast<int>(p.board[r][c].type)*4+static_cast<int>(p.board[r][c].color));
    }
    h=fnvMix(h, static_cast<int>(p.sideToMove));
    h=fnvMix(h, (p.whiteKingSideCastle?1:0)|(p.whiteQueenSideCastle?2:0)|(p.blackKingSideCastle?4:0)|(p.blackQueenSideCastle?8:0));
    h=fnvMix(h, (p.enPassantRow+1)*16+(p.enPassantCol+1));
    return h;
}

void addMove(std::vector<Move>& out,int fr,int fc,int tr,int tc,PieceType promo=PieceType::NONE){
    Move m{fr,fc,tr,tc,promo}; out.push_back(m);
}

void addPawnMoves(const Position&p,std::vector<Move>& out,int r,int c){
    Color side=p.board[r][c].color; int dir=side==Color::WHITE?-1:1; int start=side==Color::WHITE?6:1; int promoRank=side==Color::WHITE?0:7;
    int nr=r+dir;
    if(isInside(nr,c) && p.board[nr][c].type==PieceType::NONE){
        if(nr==promoRank){for(PieceType pt:{PieceType::QUEEN,PieceType::ROOK,PieceType::BISHOP,PieceType::KNIGHT}) addMove(out,r,c,nr,c,pt);} else addMove(out,r,c,nr,c);
        int nr2=r+2*dir; if(r==start && p.board[nr2][c].type==PieceType::NONE) addMove(out,r,c,nr2,c);
    }
    for(int dc:{-1,1}){ int nc=c+dc; if(!isInside(nr,nc)) continue;
        if(p.board[nr][nc].type!=PieceType::NONE && p.board[nr][nc].color==opposite(side) && p.board[nr][nc].type!=PieceType::KING){
            if(nr==promoRank){for(PieceType pt:{PieceType::QUEEN,PieceType::ROOK,PieceType::BISHOP,PieceType::KNIGHT}) addMove(out,r,c,nr,nc,pt);} else addMove(out,r,c,nr,nc);
        }
        if(nr==p.enPassantRow && nc==p.enPassantCol) addMove(out,r,c,nr,nc);
    }
}

void addKnightMoves(const Position&p,std::vector<Move>&out,int r,int c){
    static const int d[8][2]={{-2,-1},{-2,1},{-1,-2},{-1,2},{1,-2},{1,2},{2,-1},{2,1}};
    for(auto&x:d){int nr=r+x[0],nc=c+x[1];if(!isInside(nr,nc))continue;if(p.board[nr][nc].color!=p.board[r][c].color)addMove(out,r,c,nr,nc);}
}

void addSliding(const Position&p,std::vector<Move>&out,int r,int c,const int dirs[][2],int n){
    for(int i=0;i<n;i++){int nr=r+dirs[i][0],nc=c+dirs[i][1];while(isInside(nr,nc)){if(p.board[nr][nc].type==PieceType::NONE)addMove(out,r,c,nr,nc);else{if(p.board[nr][nc].color!=p.board[r][c].color)addMove(out,r,c,nr,nc);break;}nr+=dirs[i][0];nc+=dirs[i][1];}}
}

void addKingMoves(const Position&p,std::vector<Move>&out,int r,int c){
    for(int dr=-1;dr<=1;dr++)for(int dc=-1;dc<=1;dc++){if(dr==0&&dc==0)continue;int nr=r+dr,nc=c+dc;if(isInside(nr,nc)&&p.board[nr][nc].color!=p.board[r][c].color)addMove(out,r,c,nr,nc);}
    Color s=p.board[r][c].color;
    if(s==Color::WHITE&&r==7&&c==4&&!isInCheck(p,Color::WHITE)){
        if(p.whiteKingSideCastle&&p.board[7][5].type==PieceType::NONE&&p.board[7][6].type==PieceType::NONE&&p.board[7][7].type==PieceType::ROOK&&p.board[7][7].color==Color::WHITE&&!isSquareAttacked(p,7,5,Color::BLACK)&&!isSquareAttacked(p,7,6,Color::BLACK)) addMove(out,7,4,7,6);
        if(p.whiteQueenSideCastle&&p.board[7][1].type==PieceType::NONE&&p.board[7][2].type==PieceType::NONE&&p.board[7][3].type==PieceType::NONE&&p.board[7][0].type==PieceType::ROOK&&p.board[7][0].color==Color::WHITE&&!isSquareAttacked(p,7,3,Color::BLACK)&&!isSquareAttacked(p,7,2,Color::BLACK)) addMove(out,7,4,7,2);
    }
    if(s==Color::BLACK&&r==0&&c==4&&!isInCheck(p,Color::BLACK)){
        if(p.blackKingSideCastle&&p.board[0][5].type==PieceType::NONE&&p.board[0][6].type==PieceType::NONE&&p.board[0][7].type==PieceType::ROOK&&p.board[0][7].color==Color::BLACK&&!isSquareAttacked(p,0,5,Color::WHITE)&&!isSquareAttacked(p,0,6,Color::WHITE)) addMove(out,0,4,0,6);
        if(p.blackQueenSideCastle&&p.board[0][1].type==PieceType::NONE&&p.board[0][2].type==PieceType::NONE&&p.board[0][3].type==PieceType::NONE&&p.board[0][0].type==PieceType::ROOK&&p.board[0][0].color==Color::BLACK&&!isSquareAttacked(p,0,3,Color::WHITE)&&!isSquareAttacked(p,0,2,Color::WHITE)) addMove(out,0,4,0,2);
    }
}

void clearSquare(Position&p,int r,int c){p.board[r][c]={PieceType::NONE,Color::NONE};}

int moveScore(const Position&p,const Move&m){
    Piece victim=p.board[m.toRow][m.toCol]; Piece attacker=p.board[m.fromRow][m.fromCol]; int s=0;
    if(victim.type!=PieceType::NONE) s += 10*PIECE_VALUES[static_cast<int>(victim.type)]-PIECE_VALUES[static_cast<int>(attacker.type)];
    if(m.promotion!=PieceType::NONE) s+=PIECE_VALUES[static_cast<int>(m.promotion)]+500;
    if(attacker.type==PieceType::KING && std::abs(m.toCol-m.fromCol)==2) s+=50;
    return s;
}

void orderMoves(const Position&p,std::vector<Move>&moves,const Move*ttMove=nullptr){
    std::stable_sort(moves.begin(),moves.end(),[&](const Move&a,const Move&b){
        int sa=(ttMove&&sameMove(a,*ttMove)?100000000:0)+moveScore(p,a);
        int sb=(ttMove&&sameMove(b,*ttMove)?100000000:0)+moveScore(p,b);
        return sa>sb;
    });
}

int evaluateImpl(const Position&p){
    int score=0;
    int whiteBishops=0, blackBishops=0;
    for(int r=0;r<8;r++)for(int c=0;c<8;c++){
        Piece pc=p.board[r][c]; if(pc.type==PieceType::NONE) continue;
        int v=PIECE_VALUES[static_cast<int>(pc.type)];
        int positional=0;
        if(pc.type==PieceType::PAWN){ positional=(pc.color==Color::WHITE?(6-r):(r-1))*4; }
        else if(pc.type==PieceType::KNIGHT||pc.type==PieceType::BISHOP){ int center=3-std::max(std::abs(3-r),std::abs(3-c)); positional=center*10; }
        else if(pc.type==PieceType::KING){ positional=(pc.color==Color::WHITE?(7-r):r)*2; }
        if(pc.type==PieceType::BISHOP){ if(pc.color==Color::WHITE)whiteBishops++;else blackBishops++; }
        score += (pc.color==Color::WHITE?1:-1)*(v+positional);
    }
    if(whiteBishops>=2) score+=30;
    if(blackBishops>=2) score-=30;
    return score;
}

int quiescence(Position&p,int alpha,int beta){
    if(Clock::now()>=deadline){stopSearch=true;return 0;}
    nodes++;
    int stand=evaluateImpl(p); if(stand>=beta)return beta; if(stand>alpha)alpha=stand;
    auto moves=generateLegalMoves(p); orderMoves(p,moves);
    for(const auto&m:moves){ if(p.board[m.toRow][m.toCol].type==PieceType::NONE&&m.promotion==PieceType::NONE)continue;
        StateInfo st{};makeMove(p,m,st);int score=-quiescence(p,-beta,-alpha);undoMove(p,m,st);if(stopSearch)return 0;if(score>=beta)return beta;if(score>alpha)alpha=score;
    }
    return alpha;
}

int negamax(Position&p,int depth,int alpha,int beta){
    if(Clock::now()>=deadline){stopSearch=true;return 0;}
    nodes++;
    bool inCheck=isInCheck(p,p.sideToMove);
    if(depth<=0)return quiescence(p,alpha,beta);
    auto moves=generateLegalMoves(p);
    if(moves.empty()) return inCheck ? -MATE : 0;
    std::uint64_t key=hashPosition(p);
    auto it=tt.find(key); Move ttMove{}; bool hasTT=false;
    if(it!=tt.end()){ttMove=it->second.best;hasTT=true;if(it->second.depth>=depth){int s=it->second.score;if(it->second.flag==TT_EXACT)return s;if(it->second.flag==TT_LOWER)alpha=std::max(alpha,s);else if(it->second.flag==TT_UPPER)beta=std::min(beta,s);if(alpha>=beta)return s;}}
    orderMoves(p,moves,hasTT?&ttMove:nullptr);
    int originalAlpha=alpha; int best=-INF; Move bestMove=moves[0];
    for(const auto&m:moves){StateInfo st{};makeMove(p,m,st);int score=-negamax(p,depth-1,-beta,-alpha);undoMove(p,m,st);if(stopSearch)return 0;if(score>best){best=score;bestMove=m;}if(score>alpha)alpha=score;if(alpha>=beta)break;}
    TTFlag flag=best<=originalAlpha?TT_UPPER:(best>=beta?TT_LOWER:TT_EXACT);tt[key]=TTEntry{depth,best,flag,bestMove};return best;
}
}

Position startPosition(){
    Position p{}; for(auto&r:p.board)for(auto&x:r)x={PieceType::NONE,Color::NONE};
    auto put=[&](int r,int c,PieceType t,Color col){p.board[r][c]={t,col};};
    PieceType back[8]={PieceType::ROOK,PieceType::KNIGHT,PieceType::BISHOP,PieceType::QUEEN,PieceType::KING,PieceType::BISHOP,PieceType::KNIGHT,PieceType::ROOK};
    for(int c=0;c<8;c++){put(0,c,back[c],Color::BLACK);put(1,c,PieceType::PAWN,Color::BLACK);put(6,c,PieceType::PAWN,Color::WHITE);put(7,c,back[c],Color::WHITE);}return p;
}

bool isInside(int r,int c){return r>=0&&r<8&&c>=0&&c<8;}
int findKing(const Position&p,Color side){for(int r=0;r<8;r++)for(int c=0;c<8;c++)if(p.board[r][c].type==PieceType::KING&&p.board[r][c].color==side)return r*8+c;return -1;}

bool isSquareAttacked(const Position&p,int r,int c,Color bySide){
    int pawnDir=bySide==Color::WHITE?1:-1; // attacker pawn comes from opposite direction
    int pr=r+pawnDir; for(int dc:{-1,1}){int pc=c+dc;if(isInside(pr,pc)&&p.board[pr][pc].color==bySide&&p.board[pr][pc].type==PieceType::PAWN)return true;}
    static const int kd[8][2]={{-2,-1},{-2,1},{-1,-2},{-1,2},{1,-2},{1,2},{2,-1},{2,1}};for(auto&d:kd){int rr=r+d[0],cc=c+d[1];if(isInside(rr,cc)&&p.board[rr][cc].color==bySide&&p.board[rr][cc].type==PieceType::KNIGHT)return true;}
    const int diag[4][2]={{-1,-1},{-1,1},{1,-1},{1,1}};for(auto&d:diag){int rr=r+d[0],cc=c+d[1];while(isInside(rr,cc)){Piece pc=p.board[rr][cc];if(pc.type!=PieceType::NONE){if(pc.color==bySide&&(pc.type==PieceType::BISHOP||pc.type==PieceType::QUEEN))return true;break;}rr+=d[0];cc+=d[1];}}
    const int ortho[4][2]={{-1,0},{1,0},{0,-1},{0,1}};for(auto&d:ortho){int rr=r+d[0],cc=c+d[1];while(isInside(rr,cc)){Piece pc=p.board[rr][cc];if(pc.type!=PieceType::NONE){if(pc.color==bySide&&(pc.type==PieceType::ROOK||pc.type==PieceType::QUEEN))return true;break;}rr+=d[0];cc+=d[1];}}
    for(int dr=-1;dr<=1;dr++)for(int dc=-1;dc<=1;dc++)if((dr||dc)&&isInside(r+dr,c+dc)&&p.board[r+dr][c+dc].color==bySide&&p.board[r+dr][c+dc].type==PieceType::KING)return true;
    return false;
}

bool isInCheck(const Position&p,Color side){int k=findKing(p,side);if(k<0)return true;return isSquareAttacked(p,k/8,k%8,opposite(side));}

std::vector<Move> generatePseudoLegalMoves(const Position&p){
    std::vector<Move> out; const int bishopDirs[4][2]={{-1,-1},{-1,1},{1,-1},{1,1}};const int rookDirs[4][2]={{-1,0},{1,0},{0,-1},{0,1}};
    for(int r=0;r<8;r++)for(int c=0;c<8;c++){Piece pc=p.board[r][c];if(pc.color!=p.sideToMove)continue;switch(pc.type){case PieceType::PAWN:addPawnMoves(p,out,r,c);break;case PieceType::KNIGHT:addKnightMoves(p,out,r,c);break;case PieceType::BISHOP:addSliding(p,out,r,c,bishopDirs,4);break;case PieceType::ROOK:addSliding(p,out,r,c,rookDirs,4);break;case PieceType::QUEEN:addSliding(p,out,r,c,bishopDirs,4);addSliding(p,out,r,c,rookDirs,4);break;case PieceType::KING:addKingMoves(p,out,r,c);break;default:break;}}
    return out;
}

std::vector<Move> generateLegalMoves(Position&p){
    std::vector<Move> legal; auto pseudo=generatePseudoLegalMoves(p); legal.reserve(pseudo.size());
    Color moving=p.sideToMove;
    for(const auto&m:pseudo){StateInfo st{};makeMove(p,m,st);if(!isInCheck(p,moving))legal.push_back(m);undoMove(p,m,st);}return legal;
}

void makeMove(Position&p,const Move&m,StateInfo&st){
    st.captured=p.board[m.toRow][m.toCol];st.moved=p.board[m.fromRow][m.fromCol];st.wks=p.whiteKingSideCastle;st.wqs=p.whiteQueenSideCastle;st.bks=p.blackKingSideCastle;st.bqs=p.blackQueenSideCastle;st.epRow=p.enPassantRow;st.epCol=p.enPassantCol;st.halfmoveClock=p.halfmoveClock;st.fullmoveNumber=p.fullmoveNumber;st.sideToMove=p.sideToMove;
    Piece moving=st.moved; clearSquare(p,m.fromRow,m.fromCol);
    bool isPawn=moving.type==PieceType::PAWN;
    bool epCapture=isPawn && m.toRow==p.enPassantRow && m.toCol==p.enPassantCol && p.board[m.toRow][m.toCol].type==PieceType::NONE && m.fromCol!=m.toCol;
    if(epCapture){int capRow=m.toRow+(moving.color==Color::WHITE?1:-1);st.captured=p.board[capRow][m.toCol];clearSquare(p,capRow,m.toCol);}
    p.board[m.toRow][m.toCol]=moving;
    if(m.promotion!=PieceType::NONE)p.board[m.toRow][m.toCol].type=m.promotion;
    if(moving.type==PieceType::KING){if(moving.color==Color::WHITE){p.whiteKingSideCastle=p.whiteQueenSideCastle=false;if(m.fromRow==7&&m.fromCol==4&&m.toCol==6){p.board[7][5]=p.board[7][7];clearSquare(p,7,7);}if(m.fromRow==7&&m.fromCol==4&&m.toCol==2){p.board[7][3]=p.board[7][0];clearSquare(p,7,0);}}else{p.blackKingSideCastle=p.blackQueenSideCastle=false;if(m.fromRow==0&&m.fromCol==4&&m.toCol==6){p.board[0][5]=p.board[0][7];clearSquare(p,0,7);}if(m.fromRow==0&&m.fromCol==4&&m.toCol==2){p.board[0][3]=p.board[0][0];clearSquare(p,0,0);}}}
    if(moving.type==PieceType::ROOK){if(m.fromRow==7&&m.fromCol==0)p.whiteQueenSideCastle=false;if(m.fromRow==7&&m.fromCol==7)p.whiteKingSideCastle=false;if(m.fromRow==0&&m.fromCol==0)p.blackQueenSideCastle=false;if(m.fromRow==0&&m.fromCol==7)p.blackKingSideCastle=false;}
    if(st.captured.type==PieceType::ROOK){if(m.toRow==7&&m.toCol==0)p.whiteQueenSideCastle=false;if(m.toRow==7&&m.toCol==7)p.whiteKingSideCastle=false;if(m.toRow==0&&m.toCol==0)p.blackQueenSideCastle=false;if(m.toRow==0&&m.toCol==7)p.blackKingSideCastle=false;}
    p.enPassantRow=p.enPassantCol=-1;if(isPawn&&std::abs(m.toRow-m.fromRow)==2){p.enPassantRow=(m.toRow+m.fromRow)/2;p.enPassantCol=m.fromCol;}
    p.halfmoveClock=(isPawn||st.captured.type!=PieceType::NONE)?0:p.halfmoveClock+1;if(p.sideToMove==Color::BLACK)p.fullmoveNumber++;p.sideToMove=opposite(p.sideToMove);
}

void undoMove(Position&p,const Move&m,const StateInfo&st){
    p.sideToMove=st.sideToMove;p.whiteKingSideCastle=st.wks;p.whiteQueenSideCastle=st.wqs;p.blackKingSideCastle=st.bks;p.blackQueenSideCastle=st.bqs;p.enPassantRow=st.epRow;p.enPassantCol=st.epCol;p.halfmoveClock=st.halfmoveClock;p.fullmoveNumber=st.fullmoveNumber;
    Piece moving=p.board[m.toRow][m.toCol]; if(m.promotion!=PieceType::NONE)moving.type=PieceType::PAWN; p.board[m.fromRow][m.fromCol]=moving; p.board[m.toRow][m.toCol]=st.captured;
    if(moving.type==PieceType::PAWN&&m.fromCol!=m.toCol&&st.captured.type==PieceType::PAWN&&m.toRow==st.epRow&&m.toCol==st.epCol&&st.epRow>=0){clearSquare(p,m.toRow,m.toCol);int capRow=m.toRow+(moving.color==Color::WHITE?1:-1);p.board[capRow][m.toCol]=st.captured;}
    if(moving.type==PieceType::KING&&std::abs(m.toCol-m.fromCol)==2){if(m.toCol==6){p.board[m.fromRow][7]=p.board[m.fromRow][5];clearSquare(p,m.fromRow,5);}else{p.board[m.fromRow][0]=p.board[m.fromRow][3];clearSquare(p,m.fromRow,3);}}
}

std::uint64_t positionKey(const Position&p){return hashPosition(p);}

std::uint64_t perft(Position&p,int depth){if(depth==0)return 1;auto moves=generateLegalMoves(p);if(depth==1)return moves.size();std::uint64_t total=0;for(const auto&m:moves){StateInfo st{};makeMove(p,m,st);total+=perft(p,depth-1);undoMove(p,m,st);}return total;}

int evaluate(const Position&p){return p.sideToMove==Color::WHITE?evaluateImpl(p):-evaluateImpl(p);}

SearchResult searchBestMove(Position&p,int maxDepth,int timeLimitMs){
    tt.clear();nodes=0;stopSearch=false;deadline=Clock::now()+std::chrono::milliseconds(std::max(1,timeLimitMs));
    SearchResult result;auto moves=generateLegalMoves(p);if(moves.empty())return result;result.move=moves[0];
    for(int depth=1;depth<=maxDepth;depth++){if(Clock::now()>=deadline)break;int best=-INF;Move bestMove=result.move;orderMoves(p,moves);for(const auto&m:moves){if(Clock::now()>=deadline){stopSearch=true;break;}StateInfo st{};makeMove(p,m,st);int score=-negamax(p,depth-1,-INF,INF);undoMove(p,m,st);if(stopSearch)break;if(score>best){best=score;bestMove=m;}}
        if(stopSearch) break;
        result.move=bestMove;
        result.score=best;
        result.depth=depth;
        result.nodes=nodes;
    }
    return result;
}

std::string moveToString(const Move&m){std::string s; s.push_back(char('a'+m.fromCol));s.push_back(char('8'-m.fromRow));s.push_back(char('a'+m.toCol));s.push_back(char('8'-m.toRow));if(m.promotion!=PieceType::NONE){char x='q';if(m.promotion==PieceType::ROOK)x='r';else if(m.promotion==PieceType::BISHOP)x='b';else if(m.promotion==PieceType::KNIGHT)x='n';s.push_back(x);}return s;}
std::string colorName(Color c){return c==Color::WHITE?"White":c==Color::BLACK?"Black":"None";}
std::string pieceName(PieceType t){switch(t){case PieceType::PAWN:return"Pawn";case PieceType::KNIGHT:return"Knight";case PieceType::BISHOP:return"Bishop";case PieceType::ROOK:return"Rook";case PieceType::QUEEN:return"Queen";case PieceType::KING:return"King";default:return"None";}}

std::string positionToFEN(const Position&p){
    std::ostringstream os;for(int r=0;r<8;r++){int empty=0;for(int c=0;c<8;c++){Piece pc=p.board[r][c];if(pc.type==PieceType::NONE)empty++;else{if(empty){os<<empty;empty=0;}os<<pieceChar(pc);}}if(empty)os<<empty;if(r<7)os<<'/';}
    os<<' '<<(p.sideToMove==Color::WHITE?'w':'b')<<' ';std::string cast="";if(p.whiteKingSideCastle)cast+='K';if(p.whiteQueenSideCastle)cast+='Q';if(p.blackKingSideCastle)cast+='k';if(p.blackQueenSideCastle)cast+='q';os<<(cast.empty()?"-":cast)<<' ';if(p.enPassantRow>=0)os<<char('a'+p.enPassantCol)<<char('8'-p.enPassantRow);else os<<'-';os<<' '<<p.halfmoveClock<<' '<<p.fullmoveNumber;return os.str();
}

bool positionFromFEN(const std::string&fen,Position&p){
    std::istringstream is(fen);std::string boardPart,side,cast,ep;int half=0,full=1;if(!(is>>boardPart>>side>>cast>>ep>>half>>full))return false;Position q{};for(auto&r:q.board)for(auto&pc:r)pc={PieceType::NONE,Color::NONE};int r=0,c=0;for(char ch:boardPart){if(ch=='/'){if(c!=8)return false;r++;c=0;continue;}if(std::isdigit((unsigned char)ch)){c+=ch-'0';continue;}if(r>=8||c>=8)return false;Color col=std::isupper((unsigned char)ch)?Color::WHITE:Color::BLACK;char lo=(char)std::tolower((unsigned char)ch);PieceType pt=PieceType::NONE;switch(lo){case'p':pt=PieceType::PAWN;break;case'n':pt=PieceType::KNIGHT;break;case'b':pt=PieceType::BISHOP;break;case'r':pt=PieceType::ROOK;break;case'q':pt=PieceType::QUEEN;break;case'k':pt=PieceType::KING;break;default:return false;}q.board[r][c++]={pt,col};}if(r!=7||c!=8)return false;q.sideToMove=side=="w"?Color::WHITE:side=="b"?Color::BLACK:Color::NONE;if(q.sideToMove==Color::NONE)return false;q.whiteKingSideCastle=cast.find('K')!=std::string::npos;q.whiteQueenSideCastle=cast.find('Q')!=std::string::npos;q.blackKingSideCastle=cast.find('k')!=std::string::npos;q.blackQueenSideCastle=cast.find('q')!=std::string::npos;q.enPassantRow=q.enPassantCol=-1;if(ep!="-"){if(ep.size()!=2||ep[0]<'a'||ep[0]>'h'||ep[1]<'1'||ep[1]>'8')return false;q.enPassantCol=ep[0]-'a';q.enPassantRow='8'-ep[1];}q.halfmoveClock=half;q.fullmoveNumber=full;p=q;return true;}
