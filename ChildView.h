#pragma once

class CChildView : public CWnd
{
    // 생성입니다.
public:
    CChildView();

    // ==== 그래프용 자료구조 ====================================
public:
    struct Node {
        int x;
        int y;
    };

    static const int MAX_NODES = 200;

    // 지도 이미지
    CImage m_mapImg;

    // 그래프 데이터
    std::vector<Node> m_nodes;          // 노드 리스트
    double m_adj[MAX_NODES][MAX_NODES]; // 인접 행렬 (거리)

    // 상태 변수
    int m_selectedForEdge;              // Ctrl로 첫 번째 점 선택
    int m_startNode;                    // Alt 시작점
    int m_endNode;                      // Alt 도착점
    std::vector<int> m_shortestPath;    // 최단경로 노드 인덱스들

    // ==== 내부 함수 ============================================
protected:
    void InitGraph();
    void DrawMap(CDC* pDC);
    void DrawEdges(CDC* pDC);
    void DrawNodes(CDC* pDC);
    void DrawShortestPath(CDC* pDC);

    int  FindNearestNode(CPoint pt);
    double Distance(const Node& a, const Node& b);
    std::vector<int> RunDijkstra(int start, int goal);
    bool IsInShortestPath(int idx);

    // 재정의입니다.
protected:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

    // 구현입니다.
public:
    virtual ~CChildView();

protected:
    DECLARE_MESSAGE_MAP()

    // 메시지 핸들러 ---------------------------------------------
public:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnPaint();
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
};


