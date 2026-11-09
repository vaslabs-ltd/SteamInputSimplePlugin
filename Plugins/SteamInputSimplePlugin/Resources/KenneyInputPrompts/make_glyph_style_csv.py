#!/usr/bin/env python3
"""Writes DT_SteamInputGlyphs.csv: one FRichTextStyleRow per controller family and prompt button, named
Glyph.<Family>.<Button> (what USteamInputPromptSettings expects by default). Import it in the editor as a Data Table
of Rich Text Style Row into the plugin's Content folder.

    python3 make_glyph_style_csv.py [--size 69] [--out DT_SteamInputGlyphs.csv]

The font assets it points at: /SteamInputSimplePlugin/Content/UI/Fonts/F_KenneyInput<Family>.
"""
import argparse, csv, re

# A full exported FTextBlockStyle (Roboto, white); fonts, sizes and colours are swapped in per row.
TEMPLATE = '(Font=(FontObject="/Script/Engine.Font\'/Engine/EngineFonts/Roboto.Roboto\'",FontMaterial=None,OutlineSettings=(OutlineSize=0,bMiteredCorners=False,bSeparateFillAlpha=False,bApplyOutlineToDropShadows=False,OutlineMaterial=None,OutlineColor=(R=0.000000,G=0.000000,B=0.000000,A=1.000000)),TypefaceFontName="Regular",Size=31.500000,LetterSpacing=0,SkewAmount=0.000000,bForceMonospaced=False,bMaterialIsStencil=False,MonospacedWidth=1.000000),ColorAndOpacity=(SpecifiedColor=(R=1.000000,G=1.000000,B=1.000000,A=1.000000),ColorUseRule=UseColor_Specified),ShadowOffset=(X=0.000000,Y=0.000000),ShadowColorAndOpacity=(R=0.000000,G=0.000000,B=0.000000,A=1.000000),SelectedBackgroundColor=(SpecifiedColor=(R=1.000000,G=0.000000,B=1.000000,A=1.000000),ColorUseRule=UseColor_Foreground),HighlightColor=(SpecifiedColor=(R=0.000000,G=0.000000,B=0.000000,A=1.000000),ColorUseRule=UseColor_Specified),HighlightShape=(TintColor=(SpecifiedColor=(R=1.000000,G=1.000000,B=1.000000,A=1.000000),ColorUseRule=UseColor_Specified),DrawAs=Image,Tiling=NoTile,Mirroring=NoMirror,ImageType=NoImage,ImageSize=(X=32.000000,Y=32.000000),Margin=(Left=0.000000,Top=0.000000,Right=0.000000,Bottom=0.000000),ResourceObject=None,OutlineSettings=(CornerRadii=(X=0.000000,Y=0.000000,Z=0.000000,W=0.000000),Color=(SpecifiedColor=(R=0.000000,G=0.000000,B=0.000000,A=0.000000),ColorUseRule=UseColor_Specified),Width=0.000000,RoundingType=HalfHeightRadius,bUseBrushTransparency=False),UVRegion=(Min=(X=0.000000,Y=0.000000),Max=(X=0.000000,Y=0.000000),bIsValid=False),bIsDynamicallyLoaded=False,ResourceName=""),StrikeBrush=(TintColor=(SpecifiedColor=(R=1.000000,G=1.000000,B=1.000000,A=1.000000),ColorUseRule=UseColor_Specified),DrawAs=Image,Tiling=NoTile,Mirroring=NoMirror,ImageType=NoImage,ImageSize=(X=32.000000,Y=32.000000),Margin=(Left=0.000000,Top=0.000000,Right=0.000000,Bottom=0.000000),ResourceObject=None,OutlineSettings=(CornerRadii=(X=0.000000,Y=0.000000,Z=0.000000,W=0.000000),Color=(SpecifiedColor=(R=0.000000,G=0.000000,B=0.000000,A=0.000000),ColorUseRule=UseColor_Specified),Width=0.000000,RoundingType=HalfHeightRadius,bUseBrushTransparency=False),UVRegion=(Min=(X=0.000000,Y=0.000000),Max=(X=0.000000,Y=0.000000),bIsValid=False),bIsDynamicallyLoaded=False,ResourceName=""),UnderlineBrush=(TintColor=(SpecifiedColor=(R=1.000000,G=1.000000,B=1.000000,A=1.000000),ColorUseRule=UseColor_Specified),DrawAs=Image,Tiling=NoTile,Mirroring=NoMirror,ImageType=NoImage,ImageSize=(X=32.000000,Y=32.000000),Margin=(Left=0.000000,Top=0.000000,Right=0.000000,Bottom=0.000000),ResourceObject=None,OutlineSettings=(CornerRadii=(X=0.000000,Y=0.000000,Z=0.000000,W=0.000000),Color=(SpecifiedColor=(R=0.000000,G=0.000000,B=0.000000,A=0.000000),ColorUseRule=UseColor_Specified),Width=0.000000,RoundingType=HalfHeightRadius,bUseBrushTransparency=False),UVRegion=(Min=(X=0.000000,Y=0.000000),Max=(X=0.000000,Y=0.000000),bIsValid=False),bIsDynamicallyLoaded=False,ResourceName=""),TransformPolicy=None,OverflowPolicy=Clip)'

BUTTONS = ["A", "B", "X", "Y", "LB", "RB", "LT", "RT", "Start", "Back", "DPadLR", "DPadUD", "DPad", "DPadUp", "DPadDown",
           "DPadLeft", "DPadRight", "LS", "RS", "L3", "R3"]

NEUTRAL = (0.411458, 0.396519, 0.402166)

# Family key -> (font asset, buttons it has, face button colours by token).
FAMILIES = {
    "Xbox": ("F_KenneyInputXbox", BUTTONS, {
        "A": (0.0, 0.15625, 0.019717), "B": (0.338542, 0.0, 0.017565), "X": (0.067026, 0.047521, 0.341146), "Y": (0.286458, 0.213295, 0.017795)}),
    "PS": ("F_KenneyInputPlayStation", BUTTONS, {
        "A": (0.091047, 0.083711, 0.567708), "B": (0.359375, 0.061178, 0.068385), "X": (0.359375, 0.045847, 0.345804), "Y": (0.133828, 0.333333, 0.202077)}),
    "Switch": ("F_KenneyInputSwitch", BUTTONS, {}),
    "SteamDeck": ("F_KenneyInputSteamDeck", BUTTONS, {}),
    "SteamController": ("F_KenneyInputSteamController", [b for b in BUTTONS if b != "R3"], {}),
    # Single Joy-Con held sideways; buttons it lacks use the Switch rows.
    "SwitchJoyCon": ("F_KenneyInputSwitch", ["A", "B", "X", "Y", "LB", "RB", "Start", "Back", "LS", "L3"], {}),
}


def style(font, size, color):
    s = TEMPLATE
    s = re.sub(r"FontObject=\"[^\"]*\"", "FontObject=\"/Script/Engine.Font'/SteamInputSimplePlugin/Content/UI/Fonts/%s.%s'\"" % (font, font), s, count=1)
    s = re.sub(r'TypefaceFontName="[^"]*"', 'TypefaceFontName="Default"', s, count=1)
    s = re.sub(r",Size=[\d.]+,LetterSpacing", ",Size=%f,LetterSpacing" % size, s, count=1)
    s = re.sub(r"ColorAndOpacity=\(SpecifiedColor=\(R=[\d.]+,G=[\d.]+,B=[\d.]+,A=[\d.]+\)",
               "ColorAndOpacity=(SpecifiedColor=(R=%f,G=%f,B=%f,A=1.000000)" % color, s, count=1)
    return s


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--size", type=float, default=69.0)
    parser.add_argument("--out", default="DT_SteamInputGlyphs.csv")
    args = parser.parse_args()
    with open(args.out, "w", newline="") as out:
        writer = csv.writer(out, quoting=csv.QUOTE_MINIMAL, lineterminator="\n")
        writer.writerow(["---", "TextStyle"])
        for family, (font, buttons, colors) in FAMILIES.items():
            for button in buttons:
                writer.writerow(["Glyph.%s.%s" % (family, button), style(font, args.size, colors.get(button, NEUTRAL))])


if __name__ == "__main__":
    main()
