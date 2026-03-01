import sys
from z3 import Solver, Int, Or, And, sat

def solve_slitherlink(filename):
    try:
        with open(filename, 'r') as f:
            lines = [line.strip() for line in f if line.strip()]

        if not lines:
            print("エラー: 入力ファイルが空です。")
            return

        # ヘッダー情報の取得 
        header = lines[0].split(',') 
        R, C = int(header[0]), int(header[1]) 

        # 盤面のパース 
        grid = []
        for line in lines[1:]: 
            row_data = line.split(',') 
            row = []
            for val in row_data: 
                if val == '': 
                    row.append(-1)
                else:
                    row.append(int(val))
            grid.append(row)

        s = Solver()

        # 変数定義: H (水平), V (垂直)
        H = [[Int(f'h_{i}_{j}') for j in range(C)] for i in range(R + 1)]
        V = [[Int(f'v_{i}_{j}') for j in range(C + 1)] for i in range(R)]

        # 0 または 1 の値を取る制約
        for i in range(R + 1):
            for j in range(C):
                s.add(Or(H[i][j] == 0, H[i][j] == 1))
        for i in range(R):
            for j in range(C + 1):
                s.add(Or(V[i][j] == 0, V[i][j] == 1))

        # 制約1: セルの数字
        for i in range(R):
            for j in range(C):
                if grid[i][j] != -1:
                    s.add(H[i][j] + H[i+1][j] + V[i][j] + V[i][j+1] == grid[i][j])

        # 制約2: 頂点の次数は0または2
        for i in range(R + 1):
            for j in range(C + 1):
                edges = []
                if j > 0: edges.append(H[i][j-1])
                if j < C: edges.append(H[i][j])
                if i > 0: edges.append(V[i-1][j])
                if i < R: edges.append(V[i][j])
                if edges:
                    s.add(Or(sum(edges) == 0, sum(edges) == 2))

        # 探索と部分巡回路の除去ループ
        while True:
            if s.check() != sat:
                print("解が存在しません、または矛盾が含まれています。")
                return

            m = s.model()
            adj = {(i, j): [] for i in range(R + 1) for j in range(C + 1)}
            active_edges = []

            # 引かれた線をグラフとして構築
            for i in range(R + 1):
                for j in range(C):
                    if m.evaluate(H[i][j]).as_long() == 1:
                        adj[(i, j)].append((i, j + 1))
                        adj[(i, j + 1)].append((i, j))
                        
            for i in range(R):
                for j in range(C + 1):
                    if m.evaluate(V[i][j]).as_long() == 1:
                        adj[(i, j)].append((i + 1, j))
                        adj[(i + 1, j)].append((i, j))

            # 連結成分（閉路）を特定
            visited = set()
            components = []
            for node in adj:
                if node not in visited and len(adj[node]) > 0:
                    comp_edges = []
                    q = [node]
                    visited.add(node)
                    while q:
                        curr = q.pop(0)
                        for nxt in adj[curr]:
                            if nxt not in visited:
                                visited.add(nxt)
                                q.append(nxt)
                            edge_nodes = tuple(sorted([curr, nxt]))
                            comp_edges.append(edge_nodes)
                    components.append(list(set(comp_edges)))

            if len(components) <= 1:
                # 単一の閉路であるため正解を出力
                print("=== Solution ===")
                for i in range(R):
                    row_h = "+"
                    for j in range(C):
                        row_h += "---+" if m.evaluate(H[i][j]).as_long() == 1 else "   +"
                    print(row_h)

                    row_v = ""
                    for j in range(C):
                        row_v += "| " if m.evaluate(V[i][j]).as_long() == 1 else "  "
                        cell_val = grid[i][j]
                        row_v += "  " if cell_val == -1 else str(cell_val) + " "
                    row_v += "|" if m.evaluate(V[i][C]).as_long() == 1 else " "
                    print(row_v)

                row_h = "+"
                for j in range(C):
                    row_h += "---+" if m.evaluate(H[R][j]).as_long() == 1 else "   +"
                print(row_h)
                break
            else:
                # 複数の閉路を検知した場合、一番小さい閉路を禁止する制約を追加して再評価
                min_comp = min(components, key=len)
                edge_vars = []
                for u, v in min_comp:
                    if u[0] == v[0]: # 水平
                        edge_vars.append(H[u[0]][min(u[1], v[1])])
                    else: # 垂直
                        edge_vars.append(V[min(u[0], v[0])][u[1]])
                s.add(sum(edge_vars) <= len(edge_vars) - 1)

    except Exception as e:
        # 例外時も必ず何かしらの出力をして停止する
        print(f"予期せぬエラーで停止しました: {e}")

if __name__ == "__main__":
    if len(sys.argv) > 1:
        solve_slitherlink(sys.argv[1])
    else:
        print("使い方: python script.py <問題のテキストファイル>")