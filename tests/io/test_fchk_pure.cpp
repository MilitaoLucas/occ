#include "catch.hpp"
#include <occ/io/fchkreader.h>
#include <occ/io/fchkwriter.h>
#include <sstream>
#include <fmt/ostream.h>
#include <occ/qm/hf.h>
#include <occ/qm/scf.h>
#include <occ/core/util.h>

using occ::Mat;
using occ::util::all_close;

const char* fchk_contents = R"(water                                                                   
SP        RHF                                                         6-31G(d,p)          
Number of atoms                            I                3
Info1-9                                    I   N=           9
          10          10           0           0           0         111
           1           1           2
Charge                                     I                0
Multiplicity                               I                1
Number of electrons                        I               10
Number of alpha electrons                  I                5
Number of beta electrons                   I                5
Number of basis functions                  I               24
Number of independent functions            I               24
Number of point charges in /Mol/           I                0
Number of translation vectors              I                0
Atomic numbers                             I   N=           3
           8           1           1
Nuclear charges                            R   N=           3
  8.00000000E+00  1.00000000E+00  1.00000000E+00
Current cartesian coordinates              R   N=           9
 -1.32695832E+00 -1.05938614E-01  1.87882241E-02 -1.93166520E+00  1.60017436E+00
 -2.17104966E-02  4.86644352E-01  7.95980993E-02  9.86248069E-03
Force Field                                I                0
Int Atom Types                             I   N=           3
           0           0           0
MM charges                                 R   N=           3
  0.00000000E+00  0.00000000E+00  0.00000000E+00
Integer atomic weights                     I   N=           3
          16           1           1
Real atomic weights                        R   N=           3
  1.59949146E+01  1.00782504E+00  1.00782504E+00
Atom fragment info                         I   N=           3
           0           0           0
Atom residue num                           I   N=           3
           0           0           0
Nuclear spins                              I   N=           3
           0           1           1
Nuclear ZEff                               R   N=           3
 -5.60000000E+00 -1.00000000E+00 -1.00000000E+00
Nuclear ZNuc                               R   N=           3
  8.00000000E+00  1.00000000E+00  1.00000000E+00
Nuclear QMom                               R   N=           3
  0.00000000E+00  0.00000000E+00  0.00000000E+00
Nuclear GFac                               R   N=           3
  0.00000000E+00  2.79284600E+00  2.79284600E+00
MicOpt                                     I   N=           3
          -1          -1          -1
Number of contracted shells                I               10
Number of primitive shells                 I               21
Pure/Cartesian d shells                    I                0
Pure/Cartesian f shells                    I                0
Highest angular momentum                   I                2
Largest degree of contraction              I                6
Shell types                                I   N=          10
           0          -1          -1          -2           0           0
           1           0           0           1
Number of primitives per shell             I   N=          10
           6           3           1           1           3           1
           1           3           1           1
Shell to atom map                          I   N=          10
           1           1           1           1           2           2
           2           3           3           3
Primitive exponents                        R   N=          21
  5.48467166E+03  8.25234946E+02  1.88046958E+02  5.29645000E+01  1.68975704E+01
  5.79963534E+00  1.55396162E+01  3.59993359E+00  1.01376175E+00  2.70005823E-01
  8.00000000E-01  1.87311370E+01  2.82539436E+00  6.40121692E-01  1.61277759E-01
  1.10000000E+00  1.87311370E+01  2.82539436E+00  6.40121692E-01  1.61277759E-01
  1.10000000E+00
Contraction coefficients                   R   N=          21
  1.83107443E-03  1.39501722E-02  6.84450781E-02  2.32714336E-01  4.70192898E-01
  3.58520853E-01 -1.10777550E-01 -1.48026263E-01  1.13076702E+00  1.00000000E+00
  1.00000000E+00  3.34946043E-02  2.34726953E-01  8.13757326E-01  1.00000000E+00
  1.00000000E+00  3.34946043E-02  2.34726953E-01  8.13757326E-01  1.00000000E+00
  1.00000000E+00
P(S=P) Contraction coefficients            R   N=          21
  0.00000000E+00  0.00000000E+00  0.00000000E+00  0.00000000E+00  0.00000000E+00
  0.00000000E+00  7.08742682E-02  3.39752839E-01  7.27158577E-01  1.00000000E+00
  0.00000000E+00  0.00000000E+00  0.00000000E+00  0.00000000E+00  0.00000000E+00
  0.00000000E+00  0.00000000E+00  0.00000000E+00  0.00000000E+00  0.00000000E+00
  0.00000000E+00
Coordinates of each shell                  R   N=          30
 -1.32695832E+00 -1.05938614E-01  1.87882241E-02 -1.32695832E+00 -1.05938614E-01
  1.87882241E-02 -1.32695832E+00 -1.05938614E-01  1.87882241E-02 -1.32695832E+00
 -1.05938614E-01  1.87882241E-02 -1.93166520E+00  1.60017436E+00 -2.17104966E-02
 -1.93166520E+00  1.60017436E+00 -2.17104966E-02 -1.93166520E+00  1.60017436E+00
 -2.17104966E-02  4.86644352E-01  7.95980993E-02  9.86248069E-03  4.86644352E-01
  7.95980993E-02  9.86248069E-03  4.86644352E-01  7.95980993E-02  9.86248069E-03
Constraint Structure                       R   N=           9
 -1.32695832E+00 -1.05938614E-01  1.87882241E-02 -1.93166520E+00  1.60017436E+00
 -2.17104966E-02  4.86644352E-01  7.95980993E-02  9.86248069E-03
Num ILSW                                   I              100
ILSW                                       I   N=         100
           0           0           0           0           2           0
           0           0           0           0           0          -1
           0           0           0           0           0           0
           0           0           0           0           0           0
           1           1           0           0           0           0
           0           0      100000           0          -1           0
           0           0           0           0           0           0
           0           0           0           1           0           0
           0           0           1           0           0           0
           0           0           4          41           0           0
           0           0           0           0           0           0
           0           0           0           3           0           0
           0           0           0           0           0           0
           0           0           0           0           0           0
           0           0           0           0           0           0
           0           0           0           0           0           0
           0           0           0           0
Num RLSW                                   I               41
RLSW                                       R   N=          41
  1.00000000E+00  1.00000000E+00  1.00000000E+00  1.00000000E+00  1.00000000E+00
  0.00000000E+00  0.00000000E+00  1.00000000E+00  0.00000000E+00  0.00000000E+00
  0.00000000E+00  0.00000000E+00  1.00000000E+00  0.00000000E+00  0.00000000E+00
  0.00000000E+00  0.00000000E+00  1.00000000E+00  0.00000000E+00  0.00000000E+00
  0.00000000E+00  0.00000000E+00  1.00000000E+00  0.00000000E+00  0.00000000E+00
  0.00000000E+00  0.00000000E+00  1.00000000E+00  0.00000000E+00  0.00000000E+00
  0.00000000E+00  0.00000000E+00  1.00000000E+00  0.00000000E+00  0.00000000E+00
  0.00000000E+00  0.00000000E+00  0.00000000E+00  1.00000000E+00  1.00000000E+00
  0.00000000E+00
MxBond                                     I                2
NBond                                      I   N=           3
           2           1           1
IBond                                      I   N=           6
           2           3           1           0           1           0
RBond                                      R   N=           6
  1.00000000E+00  1.00000000E+00  1.00000000E+00  0.00000000E+00  1.00000000E+00
  0.00000000E+00
Virial Ratio                               R      2.001700508404843E+00
SCF Energy                                 R     -7.602228059102917E+01
Total Energy                               R     -7.602228059102917E+01
RMS Density                                R      3.113711538125358E-09
External E-field                           R   N=          35
  0.00000000E+00  0.00000000E+00  0.00000000E+00  0.00000000E+00  0.00000000E+00
  0.00000000E+00  0.00000000E+00  0.00000000E+00  0.00000000E+00  0.00000000E+00
  0.00000000E+00  0.00000000E+00  0.00000000E+00  0.00000000E+00  0.00000000E+00
  0.00000000E+00  0.00000000E+00  0.00000000E+00  0.00000000E+00  0.00000000E+00
  0.00000000E+00  0.00000000E+00  0.00000000E+00  0.00000000E+00  0.00000000E+00
  0.00000000E+00  0.00000000E+00  0.00000000E+00  0.00000000E+00  0.00000000E+00
  0.00000000E+00  0.00000000E+00  0.00000000E+00  0.00000000E+00  0.00000000E+00
IOpCl                                      I                0
IROHF                                      I                0
Alpha Orbital Energies                     R   N=          24
 -2.05589450E+01 -1.33844050E+00 -6.99083537E-01 -5.69456117E-01 -4.96636158E-01
  2.14584623E-01  3.03887789E-01  9.99392947E-01  1.08838247E+00  1.16924736E+00
  1.19339506E+00  1.29513767E+00  1.59997737E+00  1.80542135E+00  1.82444270E+00
  1.92259977E+00  2.57752454E+00  2.58758038E+00  2.79825308E+00  2.96948666E+00
  2.99238082E+00  3.36881195E+00  3.73369557E+00  3.92411724E+00
Alpha MO coefficients                      R   N=         576
  9.95714758E-01  2.19148286E-02  1.11882321E-03  1.81673565E-03 -4.73656095E-05
 -5.92622076E-03 -5.41139116E-04 -8.30914590E-04  2.17342553E-05 -3.61628552E-04
  2.66602223E-06 -1.36187347E-05  5.57186920E-05 -1.75185134E-04 -5.18830333E-04
  1.25992249E-03 -3.35148445E-04  1.12586985E-03 -2.68789830E-05 -4.59678195E-04
  1.24904471E-03  1.12419360E-03  1.75409608E-04 -7.01805969E-06 -2.12025356E-01
  4.70848647E-01  4.35442905E-02  7.12459986E-02 -1.85671419E-03  4.40694871E-01
  1.82535625E-02  2.90395744E-02 -7.58004229E-04 -3.75515177E-03 -4.63414823E-05
 -1.79142300E-04 -6.48205845E-04  1.31116774E-03  1.45563375E-01  1.06542201E-02
  1.09764134E-02 -2.38455420E-02  5.59957431E-04  1.42840399E-01  1.07671479E-02
 -2.59618526E-02 -2.25681233E-04  6.80114345E-05 -8.81323533E-04  2.01301980E-03
  4.21752124E-01 -2.71031847E-01  5.65001864E-03  4.42234092E-03  2.34917268E-01
 -1.50622895E-01  3.13865117E-03  6.12837893E-05 -4.29461251E-04  7.24045834E-04
  3.07373123E-02  1.44694361E-02 -2.40688447E-01 -1.33825941E-01  8.15652506E-04
  2.17754461E-02 -5.37424097E-04  2.38736776E-01  1.34282685E-01 -1.92761866E-02
 -1.00882557E-02  2.94448607E-04  7.10298020E-02 -1.69017710E-01  2.97075873E-01
  4.64626515E-01 -1.21399784E-02 -3.07754879E-01  2.10638240E-01  3.29409551E-01
 -8.60701040E-03 -1.81603937E-02 -5.47897461E-04 -1.05366995E-03 -9.39869761E-03
  2.01205327E-02  1.45304715E-01  8.60384496E-02  1.62293355E-02 -4.64182919E-03
  7.50969944E-05  1.47895469E-01  8.91182209E-02 -1.14399755E-02  1.24818558E-02
 -2.79407275E-04 -5.89909141E-12  1.54916883E-11  1.53618758E-03  1.57009570E-02
  6.38505678E-01  3.76225569E-11  1.20410776E-03  1.23068591E-02  5.00479011E-01
 -1.06047130E-03  1.47160707E-02  2.34436892E-02 -5.41445201E-04  4.18346403E-04
 -1.45126752E-11 -2.60336607E-11  4.90245884E-05  5.01067038E-04  2.03767292E-02
 -1.24905162E-11 -2.15167016E-11  4.85891718E-05  4.96616751E-04  2.01957509E-02
 -8.64938453E-02  9.68177246E-02  1.21481796E-01  1.83811870E-01 -4.81223769E-03
  1.21944142E+00  2.76004516E-01  4.21992481E-01 -1.10409050E-02 -2.05499028E-02
 -1.91509637E-04 -9.40439736E-04 -2.18553740E-03  4.51710664E-03 -5.75570902E-02
 -1.00050915E+00 -4.85917978E-03  1.21753210E-02 -2.87702344E-04 -5.95549280E-02
 -1.01774266E+00  1.29835241E-02  6.26918500E-04 -4.66532252E-05  9.49669541E-04
 -6.55511585E-04  2.79495456E-01 -1.78141966E-01  3.70809842E-03 -2.08810277E-02
  7.17503789E-01 -4.59940584E-01  9.58376271E-03  5.19489641E-04 -4.67980996E-04
  7.93966718E-04  3.29254300E-02  1.58996285E-02  5.55739960E-02  1.42186112E+00
  8.12821629E-03 -1.51701438E-02  3.53480507E-04 -5.58937546E-02 -1.39404301E+00
  1.62663875E-02 -7.73407700E-04 -2.01172549E-05  8.71029572E-04 -9.87400355E-03
 -1.31628367E-01  6.69588396E-02 -1.32984251E-03  2.29348398E-02 -2.85668147E-01
  1.94987482E-01 -4.10748142E-03 -5.76816173E-03 -3.42499598E-03  5.56248868E-03
  2.47236495E-01  1.14128556E-01 -7.51420071E-01  6.12709083E-01 -1.37941311E-02
 -1.12203426E-01  2.79228768E-03  7.90780153E-01 -6.56481966E-01  1.04118410E-01
  5.85352927E-02 -1.68989182E-03  3.65253268E-02 -2.96609601E-01 -2.75783158E-01
 -4.34702267E-01  1.13529080E-02  5.95441406E-01  2.09972418E-01  2.91593103E-01
 -7.67549598E-03 -2.24140838E-01  1.72012520E-03 -8.22876061E-03  4.28130099E-02
 -1.12143919E-01  6.61483586E-01 -6.37351371E-01 -1.23696666E-01  1.62734820E-01
 -3.70407218E-03  6.19791405E-01 -6.17475842E-01  1.93687706E-01 -4.57059918E-02
  6.57922356E-04 -5.80641574E-12  3.54110795E-11 -2.31559143E-03 -2.36670323E-02
 -9.62459454E-01 -1.23694180E-11  2.50283857E-03  2.55808344E-02  1.04028742E+00
 -5.50106829E-04  7.83266823E-03  1.21416755E-02 -2.79911260E-04  2.21855942E-04
 -5.09888825E-11  1.32532164E-11 -2.32016179E-05 -2.37137506E-04 -9.64359325E-03
 -4.79410984E-11  1.88300250E-11 -1.74007843E-05 -1.77848648E-04 -7.23251294E-03
 -6.28131917E-02  4.02804770E-01 -3.98277834E-01 -6.45006934E-01  1.68190448E-02
 -8.66711741E-03  6.05158340E-01  9.95994390E-01 -2.59476187E-02  6.48449709E-02
 -1.21849430E-03  1.81264526E-03 -3.24081834E-02  6.37159330E-02 -4.23546219E-01
 -1.08673349E-02  2.05302820E-02  4.49767587E-02 -1.15537973E-03 -4.18963258E-01
  1.30694774E-02  2.80129894E-02  3.92029994E-02 -1.03140479E-03 -8.16639848E-04
  6.57709258E-03 -7.48556221E-01  4.64812491E-01 -9.62885429E-03 -1.53619940E-02
  1.49366749E+00 -9.34926151E-01  1.93963535E-02  3.79166872E-03 -1.90974878E-03
  3.26300875E-03  1.32447004E-01  6.53544176E-02  1.19292793E-01  9.63165888E-01
  7.84125494E-02 -2.02435137E-01  4.78925887E-03 -1.29417702E-01 -9.49664488E-01
  2.13471734E-01  1.61148367E-02 -9.09860330E-04  5.08700069E-02 -1.45097260E+00
 -5.41668070E-02 -8.03340454E-02  2.10574755E-03  2.45902049E+00  3.03463239E-01
  4.84930614E-01 -1.26546270E-02  8.22486044E-02 -4.52463211E-03  5.23335712E-04
 -1.00846506E-01  2.07807007E-01 -2.55473153E-01 -6.17864789E-01  1.01041493E-01
  1.76774688E-01 -4.59001472E-03 -2.53744040E-01 -6.06011312E-01  1.15176187E-01
  1.67680715E-01 -4.40039927E-03 -3.82244236E-15 -1.05428484E-12  6.02486031E-06
  6.15784615E-05  2.50419115E-03  2.24847590E-12 -5.35979870E-06 -5.47810528E-05
 -2.22776320E-03  1.28228197E-02  5.70910466E-01 -3.56740194E-01  1.01502918E-02
  1.31796189E-02 -1.45515865E-12 -1.10787639E-12 -8.48736475E-04 -8.67470545E-03
 -3.52771407E-01 -8.52403593E-14  1.91723128E-13  8.58824970E-04  8.77781724E-03
  3.56964620E-01  7.45436780E-03 -5.43121596E-01 -1.67816266E-02 -2.52737459E-02
  6.61860558E-04  1.03268988E+00  2.89904321E-01  4.62823771E-01 -1.20783946E-02
  3.67183568E-01  1.34999788E-02  2.28148159E-02  2.41298139E-01 -5.10325268E-01
 -2.76618496E-01 -2.15022314E-01 -2.85806768E-01 -7.14222040E-02  2.44390898E-03
 -2.74691791E-01 -2.09411629E-01  5.72953965E-02 -2.90500646E-01  7.00560984E-03
  2.30623450E-13 -5.00255008E-12 -6.05328594E-05 -6.18689951E-04 -2.51600617E-02
  7.22634081E-12 -4.71819881E-04 -4.82234313E-03 -1.96108650E-01 -2.97601091E-02
  4.13334135E-01  6.57867732E-01 -1.51929181E-02  1.17487566E-02 -6.52365708E-13
 -1.32153994E-12  7.37900726E-04  7.54188331E-03  3.06703300E-01 -7.87517933E-13
 -1.39965069E-12  7.33452638E-04  7.49642061E-03  3.04854483E-01  3.65529811E-04
 -5.06384389E-02 -3.32526879E-02  7.01577393E-03 -9.25160107E-05  8.32900137E-02
  4.24350527E-01 -2.13068800E-01  4.21844651E-03 -6.70804606E-02  1.90910344E-03
 -5.35983890E-03 -1.09315856E-01 -7.83215831E-02  6.13711972E-02  9.05104984E-02
 -7.12087437E-01 -2.44770632E-01  7.73216780E-03 -1.62303605E-01 -1.00162704E-01
 -1.14197183E-01  7.46220190E-01 -1.80749259E-02  2.92353932E-03 -6.69185529E-01
 -1.06630371E-01 -1.50227829E-01  3.95066954E-03  1.13998435E+00  3.21793137E-01
  5.60458160E-01 -1.45559615E-02 -8.88871411E-01  1.99105788E-03 -3.36013051E-02
  1.49836417E-01 -2.46342281E-01 -6.71141522E-01 -9.37520570E-02  9.23873042E-02
 -1.63608186E-01  3.80087593E-03 -6.76405666E-01 -6.24816079E-02 -1.94466992E-01
 -1.03800851E-01  3.02034977E-03  8.93996428E-04  2.25662953E-02 -3.00475190E-01
  1.93936584E-01 -4.04601523E-03 -4.77432913E-02  4.08205597E-01 -2.87631654E-01
  6.09080230E-03  2.87330240E-02 -1.03410629E-02  1.80877190E-02  7.20112826E-01
  3.54990818E-01  5.62341832E-01 -1.75032237E-01 -1.49655095E-01  3.78867215E-01
 -8.95634877E-03 -5.11274969E-01  1.86300029E-01 -4.00885897E-01 -2.73591019E-02
  1.63726034E-03 -4.83728320E-15 -1.16616395E-13 -4.12871787E-06 -4.21984986E-05
 -1.71607255E-03  2.64012620E-13 -1.83888546E-04 -1.87947501E-03 -7.64319951E-02
 -1.14472174E-02 -8.10912267E-01  3.47944129E-01 -1.05109424E-02 -1.91025581E-02
  2.07696707E-12 -8.00335828E-14 -1.33003582E-03 -1.35939357E-02 -5.52820132E-01
 -2.42693192E-12  4.58020051E-14  2.01600808E-03  2.06050722E-02  8.37939725E-01
 -9.83079189E-14 -4.20971285E-13 -6.00337417E-05 -6.13588603E-04 -2.49526069E-02
  1.97130435E-12 -9.65738596E-04 -9.87055243E-03 -4.01402525E-01  3.12593687E-02
 -2.46693496E-01 -7.09351415E-01  1.68603510E-02 -7.77500417E-03 -1.93532130E-12
 -1.09703778E-13  2.13056513E-03  2.17759287E-02  8.85554565E-01 -8.51787101E-13
 -1.87242809E-13  1.47480659E-03  1.50735984E-02  6.12993091E-01  1.08840332E-03
 -1.63673872E-01 -1.74013086E-02 -3.12232907E-02  8.09651717E-04  2.84632791E-01
  2.55250862E-01  4.02306352E-01 -1.05068884E-02 -8.91247636E-02 -2.13618131E-02
 -1.65442542E-02 -4.30340220E-01  8.95670196E-01 -9.63144269E-02 -6.87018685E-02
 -7.54848880E-01 -3.22984620E-01  9.75834251E-03 -9.27352064E-02 -6.89995730E-02
  3.04000850E-02 -8.08345457E-01  1.98042045E-02 -1.08198020E-01  1.55018125E-01
  3.29502170E-01  4.95448957E-01 -1.29759221E-02  1.88403766E+00  4.81463971E-01
  6.75714580E-01 -1.77742891E-02 -7.76044355E-01  1.02604958E-03 -2.90140484E-02
  1.46594642E-01 -1.87650436E-01 -9.42010039E-01 -4.26066909E-01 -4.08448525E-01
  9.21935228E-01 -2.16878444E-02 -1.01016187E+00 -4.57730842E-01  1.07340137E+00
  3.72953631E-02 -3.49960670E-03  3.68868721E-03 -2.01857169E-02  6.08113873E-01
 -4.19640563E-01  8.85596110E-03 -7.19660433E-02  1.08051959E+00 -7.19003182E-01
  1.50807699E-02  2.73912756E-02 -1.88977049E-02  3.24347560E-02  1.33370357E+00
  6.42732152E-01  1.25207669E+00  5.19116968E-01  3.21243384E-01 -1.05089516E+00
  2.50687975E-02 -1.16206687E+00 -4.87274264E-01  1.01232143E+00  1.55691815E-01
 -6.26404157E-03
Total SCF Density                          R   N=         300
  2.08289728E+00 -1.80036004E-01  5.01499492E-01  2.52221509E-02 -5.76695986E-02
  5.36057291E-01  3.98883769E-02 -9.09796135E-02  5.36991581E-02  5.89323747E-01
 -1.04154382E-03  2.37595119E-03 -6.47250261E-04  5.44177959E-03  8.15744505E-01
 -2.42405956E-01  5.18791277E-01 -1.40756605E-01 -2.25605385E-01  5.88632043E-03
  5.77959424E-01  2.06910321E-02 -5.30917873E-02  3.24896957E-01  7.10329327E-02
 -9.89786257E-04 -1.11477207E-01  1.99779052E-01  3.30923253E-02 -8.46484398E-02
  7.12331752E-02  3.92273373E-01  5.90814714E-03 -1.78481973E-01  6.90953465E-02
  2.64386709E-01 -8.63526360E-04  2.20925280E-03 -9.94710252E-04  5.90862891E-03
  6.39364638E-01  4.65710455E-03 -9.73719088E-04  5.65862067E-03  5.01127495E-01
 -1.70774936E-03  2.58703674E-03 -1.10694347E-02 -1.74785145E-02 -8.98629063E-04
  7.87297553E-03 -7.76100575E-03 -1.22264730E-02 -7.42812041E-04  6.90320390E-04
 -5.21167739E-05  1.39956923E-04 -6.46603181E-04  1.79179159E-04  1.88012113E-02
  2.92561281E-04 -3.98847251E-04  1.27929394E-04  1.47369751E-02 -1.10184017E-05
  4.34099041E-04 -1.02115503E-04  1.89798078E-04  4.10919850E-05 -6.61004068E-04
  2.99722887E-02  4.97215322E-04 -5.37724033E-05 -3.45639087E-04  2.34891027E-02
 -1.00085895E-05  6.90547214E-04  1.10254661E-03 -1.00352230E-03  2.69287077E-03
  2.02848107E-02 -2.55045134E-02 -1.13497136E-04  5.48486946E-03  1.04569722E-02
 -1.55025942E-02 -1.86242032E-04  3.51111794E-04 -3.19774411E-05  3.91604747E-05
  2.06766864E-03  1.92793993E-03 -5.51615338E-03  2.42747615E-02  1.10530412E-02
  2.04360278E-04 -1.10986805E-02  1.53236120E-02  8.98367423E-03  1.61225589E-04
 -7.39627958E-04 -2.22857283E-05 -2.29756474E-06  5.09117190E-04  1.23225056E-03
 -4.16932878E-02  8.69667346E-02 -1.04012931E-01  2.86233047E-01 -6.78827069E-03
  3.67389320E-02 -4.65556239E-02  1.76690962E-01 -4.23285042E-03 -6.39993217E-03
  3.40145424E-05 -7.06884343E-04 -1.77163500E-02 -7.36137904E-04  2.00466709E-01
  1.04495932E-02 -1.95345598E-02 -6.08321748E-02  1.54016393E-01 -3.64093104E-03
 -4.47655661E-02 -2.62424823E-02  9.76149696E-02 -2.33723070E-03 -3.22231464E-03
  1.96848687E-05 -3.78956049E-04 -9.85787008E-03 -3.82995449E-04  9.25247219E-02
  5.08541943E-02 -3.01788513E-03  4.83896290E-03  1.12860153E-02  1.62033964E-02
 -3.62954083E-04 -3.03629752E-04  7.62145713E-03  1.10857487E-02 -2.41835371E-04
 -6.71660049E-04 -1.80608207E-05 -3.46445034E-05 -2.69247689E-04  7.05632097E-04
  7.51963701E-03  2.80742853E-03  7.69305996E-04  1.16560107E-02 -2.07491644E-02
  1.35371147E-02 -1.94950862E-02  1.08707712E-03 -1.79808724E-02  7.40481912E-03
 -1.09923538E-02  7.54341658E-04  3.48474086E-04  3.34662684E-06  7.33212984E-05
  1.45638817E-03  3.80858458E-04 -1.87743994E-02 -7.13226500E-03 -6.39327802E-04
  2.13169031E-03 -2.79362409E-04  4.98583167E-04 -2.97389790E-04  1.08066229E-03
  2.60113416E-02  4.42883040E-04 -1.51320721E-04  7.45485671E-04  2.03907342E-02
 -5.01973357E-05  6.00058050E-04  9.54275028E-04 -5.72442676E-05  5.99641236E-06
  4.43573916E-04  1.68629117E-04  1.58694329E-05 -3.04477604E-05  8.31639667E-04
 -4.08978265E-02  8.54595263E-02  3.01686575E-01  2.83737176E-02 -1.42354520E-03
  3.69839571E-02  1.79686854E-01  3.38147241E-02 -1.26382000E-03 -6.41486079E-03
 -3.80360816E-04 -1.71182808E-05  1.17109729E-02  1.32349808E-02 -3.03574116E-02
 -3.54064060E-02  8.32600152E-03  2.21094003E-03 -7.43989853E-05  1.98543818E-01
  1.03449740E-02 -1.93903486E-02  1.67158252E-01  1.15623796E-02 -6.86488600E-04
 -4.41901976E-02  1.01025779E-01  1.88839639E-02 -7.07418711E-04 -3.30215318E-03
 -2.13984985E-04  2.75952974E-06  6.56596774E-03  7.49999905E-03 -3.56086776E-02
 -2.03732477E-02  3.34724713E-03  4.51010318E-03 -1.18957338E-04  9.35516117E-02
  5.21827766E-02  1.16567131E-02 -2.06094231E-02 -2.53149428E-02 -3.87545669E-03
  2.18290393E-04 -1.60249102E-02 -1.48249030E-02 -3.23851888E-03  1.63968501E-04
  6.07211540E-04  3.29349166E-05  7.74356217E-06 -9.36224684E-04 -1.08662182E-03
 -1.60481024E-03  2.64037833E-03 -9.73456571E-04  5.07438679E-04 -8.15456336E-06
 -2.00055467E-02 -7.77219858E-03  2.35545674E-03  2.23596568E-03 -4.46476035E-03
 -1.11110612E-03  1.70513538E-02  2.17950079E-04 -7.97292362E-03  5.11268476E-04
  1.12611543E-02  2.19252324E-04 -4.54072551E-04  9.62582223E-06 -1.75510938E-05
 -8.55023767E-04  2.10102676E-04  8.41771436E-03  4.84361277E-03  3.83664204E-04
 -5.43574186E-04  3.26947199E-05 -1.18948952E-03 -4.89056261E-04  1.15502806E-04
  5.15795913E-04 -8.30277645E-05  1.59373603E-04  1.50314570E-04  2.24600860E-04
  2.58000626E-02  2.34609972E-04  7.17606292E-05  2.28273977E-04  2.02216535E-02
 -3.31553570E-05  5.94451120E-04  9.47916833E-04  1.39450883E-06  1.43558114E-05
 -2.03131916E-04 -1.25457729E-04 -5.11092917E-06  3.23969295E-05  8.22764787E-04
  7.73812949E-05  3.07251862E-05 -6.54346321E-06  7.10994821E-06  8.16075595E-04
Mulliken Charges                           R   N=           3
 -6.80982819E-01  3.40141626E-01  3.40841193E-01
ONIOM Charges                              I   N=          16
           0           0           0           0           0           0
           0           0           0           0           0           0
           0           0           0           0
ONIOM Multiplicities                       I   N=          16
           1           0           0           0           0           0
           0           0           0           0           0           0
           0           0           0           0
Atom Layers                                I   N=           3
           1           1           1
Atom Modifiers                             I   N=           3
           0           0           0
Force Field                                I                0
Int Atom Modified Types                    I   N=           3
           0           0           0
Link Atoms                                 I   N=           3
           0           0           0
Atom Modified MM Charges                   R   N=           3
  0.00000000E+00  0.00000000E+00  0.00000000E+00
Link Distances                             R   N=          12
  0.00000000E+00  0.00000000E+00  0.00000000E+00  0.00000000E+00  0.00000000E+00
  0.00000000E+00  0.00000000E+00  0.00000000E+00  0.00000000E+00  0.00000000E+00
  0.00000000E+00  0.00000000E+00
Cartesian Gradient                         R   N=           9
  0.00000000E+00  0.00000000E+00  0.00000000E+00  0.00000000E+00  0.00000000E+00
  0.00000000E+00  0.00000000E+00  0.00000000E+00  0.00000000E+00
Dipole Moment                              R   N=           3
  4.64108052E-01  7.30177906E-01 -1.90717942E-02
Quadrupole Moment                          R   N=           6
  3.43355128E-02  6.97954815E-01 -7.32290328E-01 -1.43446166E+00  3.94577753E-02
 -2.22383175E-02
QEq coupling tensors                       R   N=          18
 -1.27543734E+00  6.30144800E-01 -6.51857970E-01 -7.78625351E-03  6.19427153E-02
  1.92729531E+00  1.27593211E-01  2.28494131E-01 -4.11972589E-01 -5.24054303E-03
  1.65833876E-02  2.84379379E-01 -4.67844909E-01 -6.07445883E-02  1.95363018E-01
  3.27501915E-03  2.04394117E-03  2.72481891E-01
)";

void print_shell(const libint2::Shell &shell) {
//    fmt::print("{}\n", shell);
}

void print_basis(const occ::qm::BasisSet &basis) {
    size_t idx = 0;
    for(const auto& sh: basis) {
	fmt::print("Shell {}\n", idx);
	print_shell(sh);
	idx++;
    }
}

TEST_CASE("G09 water fchk", "[read]")
{
    std::istringstream fchk(fchk_contents);
    occ::io::FchkReader reader(fchk);
    REQUIRE(reader.num_alpha() == 5);
    REQUIRE(reader.num_basis_functions() == 24);
    REQUIRE(reader.basis().primitive_exponents[0] == Approx(5.48467166e03));
    occ::io::FchkReader::FchkBasis basis = reader.basis();
    auto density = reader.scf_density_matrix();
    REQUIRE(density(0, 0) == Approx(2.08289728 * 0.5));
    auto obs = reader.basis_set();
}

void check_wavefunctions(const occ::qm::Wavefunction &wfn, const occ::qm::Wavefunction &wfn2) {
    // fmt::print("Original D\n{}\n", wfn.mo.D);
    // fmt::print("After reading D\n{}\n", wfn2.mo.D);
    CHECK(wfn2.mo.D.rows() == wfn.mo.D.rows());
    CHECK(wfn2.mo.D.cols() == wfn.mo.D.cols());
    CHECK(all_close(wfn2.mo.D, wfn.mo.D, 1e-6, 1e-6));
    // fmt::print("Original C\n{}\n", wfn.mo.C);
    // fmt::print("After reading C\n{}\n", wfn2.mo.C);

    CHECK(wfn2.mo.C.rows() == wfn.mo.C.rows());
    CHECK(wfn2.mo.C.cols() == wfn.mo.C.cols());
    CHECK(all_close(wfn2.mo.C, wfn.mo.C, 1e-6, 1e-6));

    auto check_same_position = [](const std::array<double, 3> &x1, const std::array<double, 3> &x2) {
	for(size_t i = 0; i < 3; i++) {
	    CHECK(x1[i] == Approx(x2[i]));
	}
    };

    auto check_same_coeffs = [](const auto &c1, const auto& c2) {
	for(size_t i = 0; i < c1.size(); i++) {
	    CHECK(c1[i] == Approx(c2[i]));
	}
    };

    for(size_t i = 0; i < wfn.basis.size(); i++) {
	const auto &sh1 = wfn.basis[i];
	const auto &sh2 = wfn2.basis[i];
	check_same_position(sh1.O, sh2.O);
	check_same_coeffs(sh1.alpha, sh2.alpha);
	check_same_coeffs(sh1.contr[0].coeff, sh2.contr[0].coeff);
	CHECK(sh1.contr[0].l == sh2.contr[0].l);
	CHECK(sh1.contr[0].pure == sh2.contr[0].pure);
	CHECK(sh2.contr[0].pure == true);
    }

}

TEST_CASE("our water fchk read write", "[read,write]") {
    libint2::Shell::do_enforce_unit_normalization(true);
    if (!libint2::initialized()) libint2::initialize();
    std::vector<occ::core::Atom> atoms{
        {8, -1.32695761, -0.10593856, 0.01878821},
        {1, -1.93166418, 1.60017351, -0.02171049},
        {1, 0.48664409, 0.07959806, 0.00986248}
    };

    occ::qm::BasisSet obs("6-31G**", atoms);
    obs.set_pure(true);
    occ::hf::HartreeFock hf(atoms, obs);
    occ::scf::SCF<occ::hf::HartreeFock, occ::qm::SpinorbitalKind::Restricted> scf(hf);
    scf.energy_convergence_threshold = 1e-8;
    double e = scf.compute_scf_energy();

    occ::qm::Wavefunction wfn = scf.wavefunction();

    std::string water_fchk_contents;
    {
	std::ostringstream fchk_os;
	occ::io::FchkWriter fchk_writer(fchk_os);
	fchk_writer.set_title("test water fchk");
	fchk_writer.set_method("hf");
	fchk_writer.set_basis_name("6-31G**");
	wfn.save(fchk_writer);
	fchk_writer.write();
	water_fchk_contents = fchk_os.str();
    }

    occ::qm::Wavefunction wfn2 = [&]() {
	std::istringstream fchk_is(water_fchk_contents);
	occ::io::FchkReader reader(fchk_is);
	return occ::qm::Wavefunction(reader);
    }();

    check_wavefunctions(wfn, wfn2);

    SECTION("Rotate by I") {
	occ::Mat rot = occ::Mat::Identity(3, 3);
	wfn2.apply_rotation(rot);
	check_wavefunctions(wfn, wfn2);
	fmt::print("orig MOs\n{}\n", wfn.mo.C);
	fmt::print("rot  MOs\n{}\n", wfn2.mo.C);
    }

}


