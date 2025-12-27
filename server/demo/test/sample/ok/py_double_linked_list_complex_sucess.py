# py_double_linked_list_complex_sucess.py
# 目标：覆盖 double_linked_list（insert_* / delete / reverse / delete_head/tail）

from ds4viz import double_linked_list

with double_linked_list("demo_dlist_complex") as d:
    # 10 <-> 30
    node_a = d.insert_head(10)
    node_b = d.insert_tail(30)

    # 10 <-> 20 <-> 30
    node_c = d.insert_after(node_a, 20)

    # 10 <-> 20 <-> 25 <-> 30
    node_d = d.insert_before(node_b, 25)

    # 删除中间节点 20：10 <-> 25 <-> 30
    d.delete(node_c)

    # 反转：30 <-> 25 <-> 10
    d.reverse()

    # 删除头/尾：25
    d.delete_head()
    d.delete_tail()

    # 再插入验证状态仍可用
    d.insert_head("X")
    d.insert_tail("Y")
