# slimeEngine

## TODO
1. Obj class
2. Collisions
3. Hooke's law mathy math

## Future features
1. Obj grabber
2. Mesh editor

## XPBD
### Approach
- create tetrahedral mesh (using delaunay tetrahedralisation, 
bro has a tutorial: https://youtu.be/sNxz_Ht6Y1Y?si=1TeYnT59vSePK5bV)
- create
    - one particle per vertex
    - one distance constraint per edge
    - one volume constraint per tetrahedron $$ C = 6(V -V_o) $$
