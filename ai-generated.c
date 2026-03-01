#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UNKNOWN 0
#define LINE 1
#define BLANK -1

// 盤面サイズと変数群
int R, C;
int E, V;
int *grid; // R * C のヒント数字 (-1は空白)

// グラフ構造 (インデックスベース)
int *edge_u, *edge_v;      // 辺eの両端の頂点
int *edge_c1, *edge_c2;    // 辺eの隣接セル (存在しない場合は -1)
int *v_edges, *v_deg;      // 頂点vに接続する辺のリストとその数
int *c_edges;              // セルcを構成する4辺

// 状態管理と履歴 (Trail)
int *edge_state;
int loop_closed = 0;

typedef struct {
    int type; // 0: edge_state, 1: uf_parent, 2: uf_size, 4: loop_closed
    int id;
    int old_val;
} TrailEntry;

TrailEntry *trail;
int trail_ptr = 0;

// 伝播用キュー
int *prop_queue;
int pq_head = 0, pq_tail = 0;

// Union-Find (Undo対応のため経路圧縮なし)
int *uf_parent, *uf_size;

int uf_find(int i) {
    while (i != uf_parent[i]) i = uf_parent[i];
    return i;
}

// 状態のロールバック (O(1)の巻き戻し)
void rollback(int target) {
    while (trail_ptr > target) {
        trail_ptr--;
        TrailEntry t = trail[trail_ptr];
        if (t.type == 0) edge_state[t.id] = t.old_val;
        else if (t.type == 1) uf_parent[t.id] = t.old_val;
        else if (t.type == 2) uf_size[t.id] = t.old_val;
        else if (t.type == 4) loop_closed = t.old_val;
    }
    pq_head = pq_tail = 0; // キューのクリア
}

// 辺への割り当てとキューへの追加
int assign(int e, int val) {
    if (edge_state[e] == val) return 1;
    if (edge_state[e] != UNKNOWN) return 0; // 矛盾

    trail[trail_ptr++] = (TrailEntry){0, e, UNKNOWN};
    edge_state[e] = val;
    prop_queue[pq_tail++] = e;

    // 線が引かれた場合、閉路(小ループ)形成をチェック
    if (val == LINE) {
        int u = edge_u[e], v = edge_v[e];
        int root_u = uf_find(u), root_v = uf_find(v);
        if (root_u == root_v) {
            trail[trail_ptr++] = (TrailEntry){4, 0, loop_closed};
            loop_closed = 1;
        } else {
            if (uf_size[root_u] < uf_size[root_v]) {
                int tmp = root_u; root_u = root_v; root_v = tmp;
            }
            trail[trail_ptr++] = (TrailEntry){1, root_v, uf_parent[root_v]};
            uf_parent[root_v] = root_u;
            trail[trail_ptr++] = (TrailEntry){2, root_u, uf_size[root_u]};
            uf_size[root_u] += uf_size[root_v];
        }
    }
    return 1;
}

// 局所制約の連鎖的伝播
int propagate() {
    while (pq_head < pq_tail) {
        int e = prop_queue[pq_head++];

        // 1. セルの制約検証
        for (int i = 0; i < 2; i++) {
            int c = (i == 0) ? edge_c1[e] : edge_c2[e];
            if (c == -1 || grid[c] == -1) continue;
            
            int req = grid[c];
            int lines = 0, blanks = 0, unk = 0;
            for (int j = 0; j < 4; j++) {
                int ce = c_edges[c * 4 + j];
                if (edge_state[ce] == LINE) lines++;
                else if (edge_state[ce] == BLANK) blanks++;
                else unk++;
            }
            
            if (lines > req || blanks > 4 - req) return 0;
            if (lines == req && unk > 0) {
                for (int j = 0; j < 4; j++)
                    if (edge_state[c_edges[c * 4 + j]] == UNKNOWN)
                        if (!assign(c_edges[c * 4 + j], BLANK)) return 0;
            }
            if (blanks == 4 - req && unk > 0) {
                for (int j = 0; j < 4; j++)
                    if (edge_state[c_edges[c * 4 + j]] == UNKNOWN)
                        if (!assign(c_edges[c * 4 + j], LINE)) return 0;
            }
        }

        // 2. 頂点の制約検証
        for (int i = 0; i < 2; i++) {
            int v = (i == 0) ? edge_u[e] : edge_v[e];
            int deg = v_deg[v];
            int lines = 0, blanks = 0, unk = 0;
            for (int j = 0; j < deg; j++) {
                int ve = v_edges[v * 4 + j];
                if (edge_state[ve] == LINE) lines++;
                else if (edge_state[ve] == BLANK) blanks++;
                else unk++;
            }
            
            if (lines > 2) return 0;
            if (lines == 1 && unk == 0) return 0;
            if (lines == 2 && unk > 0) {
                for (int j = 0; j < deg; j++)
                    if (edge_state[v_edges[v * 4 + j]] == UNKNOWN)
                        if (!assign(v_edges[v * 4 + j], BLANK)) return 0;
            }
            if (lines == 1 && unk == 1) {
                for (int j = 0; j < deg; j++)
                    if (edge_state[v_edges[v * 4 + j]] == UNKNOWN)
                        if (!assign(v_edges[v * 4 + j], LINE)) return 0;
            }
            if (lines == 0 && unk == 1) { // 1本だけ余ることはない
                for (int j = 0; j < deg; j++)
                    if (edge_state[v_edges[v * 4 + j]] == UNKNOWN)
                        if (!assign(v_edges[v * 4 + j], BLANK)) return 0;
            }
        }

        // 3. 閉路(ループ)完成時の強制伝播
        if (loop_closed) {
            for (int i = 0; i < E; i++) {
                if (edge_state[i] == UNKNOWN) {
                    if (!assign(i, BLANK)) return 0;
                }
            }
        }
    }
    return 1;
}

// 先読み推論（両方の仮定での共通項抽出）
int probe_and_deduce() {
    int changed = 0;
    int current_trail = trail_ptr;
    int *line_state = (int*)malloc(E * sizeof(int));
    int *common_vals = (int*)malloc(E * sizeof(int));
    
    for(int i = 0; i < E; i++) common_vals[i] = UNKNOWN;

    for (int e = 0; e < E; e++) {
        if (edge_state[e] != UNKNOWN) continue;
        
        // 仮定: LINE
        int ok_line = assign(e, LINE) && propagate();
        if (ok_line) memcpy(line_state, edge_state, E * sizeof(int));
        rollback(current_trail);
        
        // 仮定: BLANK
        int ok_blank = assign(e, BLANK) && propagate();
        if (ok_line && ok_blank) {
            // 両方で矛盾しない場合、共通で確定したマスを記録
            for (int i = 0; i < E; i++) {
                if (line_state[i] != UNKNOWN && line_state[i] == edge_state[i]) {
                    if (common_vals[i] == UNKNOWN) common_vals[i] = line_state[i];
                }
            }
        }
        rollback(current_trail);
        
        if (!ok_line && !ok_blank) {
            free(line_state); free(common_vals); return -1; // 矛盾
        } else if (ok_line && !ok_blank) {
            if (!assign(e, LINE) || !propagate()) { free(line_state); free(common_vals); return -1; }
            changed++;
            current_trail = trail_ptr;
        } else if (!ok_line && ok_blank) {
            if (!assign(e, BLANK) || !propagate()) { free(line_state); free(common_vals); return -1; }
            changed++;
            current_trail = trail_ptr;
        }
    }

    // 両方の仮定で共通して現れた結果を適用
    for (int i = 0; i < E; i++) {
        if (common_vals[i] != UNKNOWN && edge_state[i] == UNKNOWN) {
            if (!assign(i, common_vals[i]) || !propagate()) {
                free(line_state); free(common_vals); return -1;
            }
            changed++;
        }
    }

    free(line_state); free(common_vals);
    return changed;
}

// DFSによる再帰探索
int solve() {
    if (!propagate()) return 0;

    int changed;
    do {
        changed = probe_and_deduce();
        if (changed == -1) return 0;
    } while (changed > 0);

    int target = -1;
    for (int i = 0; i < E; i++) {
        if (edge_state[i] == UNKNOWN) {
            target = i;
            break;
        }
    }

    if (target == -1) return 1; // 完了

    int saved_trail = trail_ptr;
    
    if (assign(target, LINE) && solve()) return 1;
    rollback(saved_trail);

    if (assign(target, BLANK) && solve()) return 1;
    rollback(saved_trail);

    return 0;
}

// データロード
void loadinfo(char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) { fprintf(stderr, "error: cannot open %s\n", filename); exit(1); }
    
    char line[1024];
    fgets(line, sizeof(line), fp);
    sscanf(line, "%d,%d", &R, &C);
    
    grid = (int*)malloc(R * C * sizeof(int));
    for (int r = 0; r < R; r++) {
        fgets(line, sizeof(line), fp);
        int c = 0; char *p = line;
        while (c < C && *p) {
            if (*p == ',') { grid[r * C + c++] = -1; p++; }
            else if (*p >= '0' && *p <= '3') {
                grid[r * C + c++] = *p - '0'; p++;
                if (*p == ',') p++;
            } else p++;
        }
        while(c < C) grid[r * C + c++] = -1;
    }
    fclose(fp);

    V = (R + 1) * (C + 1);
    E = (R + 1) * C + R * (C + 1);
    
    edge_u = (int*)malloc(E * sizeof(int)); edge_v = (int*)malloc(E * sizeof(int));
    edge_c1 = (int*)malloc(E * sizeof(int)); edge_c2 = (int*)malloc(E * sizeof(int));
    v_edges = (int*)malloc(V * 4 * sizeof(int)); v_deg = (int*)calloc(V, sizeof(int));
    c_edges = (int*)malloc(R * C * 4 * sizeof(int));
    edge_state = (int*)calloc(E, sizeof(int));
    trail = (TrailEntry*)malloc(E * 10 * sizeof(TrailEntry));
    prop_queue = (int*)malloc(E * sizeof(int));
    uf_parent = (int*)malloc(V * sizeof(int)); uf_size = (int*)malloc(V * sizeof(int));

    for (int i = 0; i < V; i++) { uf_parent[i] = i; uf_size[i] = 1; }

    // グラフ構築 (水平辺)
    for (int r = 0; r <= R; r++) {
        for (int c = 0; c < C; c++) {
            int e = r * C + c;
            edge_u[e] = r * (C + 1) + c;
            edge_v[e] = r * (C + 1) + c + 1;
            edge_c1[e] = (r > 0) ? (r - 1) * C + c : -1;
            edge_c2[e] = (r < R) ? r * C + c : -1;
            v_edges[edge_u[e] * 4 + v_deg[edge_u[e]]++] = e;
            v_edges[edge_v[e] * 4 + v_deg[edge_v[e]]++] = e;
            if (r > 0) c_edges[((r - 1) * C + c) * 4 + 2] = e; // 下辺
            if (r < R) c_edges[(r * C + c) * 4 + 0] = e;       // 上辺
        }
    }
    // グラフ構築 (垂直辺)
    int offset = (R + 1) * C;
    for (int r = 0; r < R; r++) {
        for (int c = 0; c <= C; c++) {
            int e = offset + r * (C + 1) + c;
            edge_u[e] = r * (C + 1) + c;
            edge_v[e] = (r + 1) * (C + 1) + c;
            edge_c1[e] = (c > 0) ? r * C + (c - 1) : -1;
            edge_c2[e] = (c < C) ? r * C + c : -1;
            v_edges[edge_u[e] * 4 + v_deg[edge_u[e]]++] = e;
            v_edges[edge_v[e] * 4 + v_deg[edge_v[e]]++] = e;
            if (c > 0) c_edges[(r * C + (c - 1)) * 4 + 1] = e; // 右辺
            if (c < C) c_edges[(r * C + c) * 4 + 3] = e;       // 左辺
        }
    }
}

// 盤面描画
void print_board() {
    for (int r = 0; r <= R; r++) {
        for (int c = 0; c <= C; c++) {
            printf("+");
            if (c < C) {
                int e = r * C + c;
                if (edge_state[e] == LINE) printf("---");
                else if (edge_state[e] == BLANK) printf("   ");
                else printf("   ");
            }
        }
        printf("\n");
        if (r < R) {
            for (int c = 0; c <= C; c++) {
                int e = (R + 1) * C + r * (C + 1) + c;
                if (edge_state[e] == LINE) printf("|");
                else if (edge_state[e] == BLANK) printf(" ");
                else printf(" ");
                if (c < C) {
                    if (grid[r * C + c] >= 0) printf(" %d ", grid[r * C + c]);
                    else printf("   ");
                }
            }
            printf("\n");
        }
    }
}

int main(int argc, char** argv) {
    if (argc == 1) { fprintf(stderr, "Usage: %s <puzzle file>\n", argv[0]); exit(1); }
    loadinfo(argv[1]);
    
    if (solve()) {
        printf("Solved:\n");
        print_board();
    } else {
        printf("No solution found.\n");
    }
    return 0;
}