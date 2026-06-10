
// HW15 Haptic Paddle First Draft
// OpenSCAD conceptual CAD model for a first 3D-printable draft.
// Units: mm

$fn = 48;

module rounded_plate(x=80, y=55, z=4, r=4) {
    hull() {
        translate([ r, r, 0]) cylinder(h=z, r=r);
        translate([ x-r, r, 0]) cylinder(h=z, r=r);
        translate([ r, y-r, 0]) cylinder(h=z, r=r);
        translate([ x-r, y-r, 0]) cylinder(h=z, r=r);
    }
}

module servo_base_plate() {
    difference() {
        rounded_plate(90, 65, 4, 4);
        // servo mounting clearance holes
        translate([25, 18, -1]) cylinder(h=6, r=2.3);
        translate([25, 47, -1]) cylinder(h=6, r=2.3);
        translate([60, 18, -1]) cylinder(h=6, r=2.3);
        translate([60, 47, -1]) cylinder(h=6, r=2.3);
        // base mounting holes
        translate([8, 8, -1]) cylinder(h=6, r=2);
        translate([82, 8, -1]) cylinder(h=6, r=2);
        translate([8, 57, -1]) cylinder(h=6, r=2);
        translate([82, 57, -1]) cylinder(h=6, r=2);
    }
}

module paddle_arm() {
    difference() {
        union() {
            translate([0, -6, 0]) cube([95, 12, 5]);
            translate([0, 0, 0]) cylinder(h=5, r=12);
            translate([70, 0, 0]) cube([22, 18, 5], center=true);
        }
        // servo horn center
        translate([0, 0, -1]) cylinder(h=7, r=3.2);
        // horn screws
        translate([0, 7, -1]) cylinder(h=7, r=1.8);
        translate([0, -7, -1]) cylinder(h=7, r=1.8);
        // load cell screw holes
        translate([64, 5, -1]) cylinder(h=7, r=1.8);
        translate([80, 5, -1]) cylinder(h=7, r=1.8);
    }
}

module load_cell_adapter() {
    difference() {
        translate([0,0,0]) cube([45, 16, 4]);
        translate([10, 8, -1]) cylinder(h=6, r=1.8);
        translate([35, 8, -1]) cylinder(h=6, r=1.8);
    }
}

module thimble_clamp() {
    difference() {
        union() {
            cube([28, 18, 10]);
            translate([14, 9, 10]) cylinder(h=12, r=8);
        }
        translate([14, 9, 7]) cylinder(h=18, r=5.2);
        translate([14, 9, -1]) cylinder(h=13, r=1.9);
        translate([14, -1, 15]) cube([2, 20, 12], center=true);
    }
}

module as5600_mount() {
    difference() {
        union() {
            cube([28, 24, 3]);
            translate([3, 0, 0]) cube([3, 24, 20]);
            translate([22, 0, 0]) cube([3, 24, 20]);
        }
        translate([14, 12, -1]) cylinder(h=5, r=4.5); // magnet clearance
        translate([6, 5, -1]) cylinder(h=5, r=1.7);
        translate([22, 19, -1]) cylinder(h=5, r=1.7);
    }
}

// Assembly preview
servo_base_plate();

translate([45, 32, 9]) rotate([0,0,25]) paddle_arm();
translate([105, 35, 8]) load_cell_adapter();
translate([120, 55, 8]) thimble_clamp();
translate([40, 75, 4]) as5600_mount();
