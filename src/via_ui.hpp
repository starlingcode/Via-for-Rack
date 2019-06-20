#include "starling.hpp"
#include "color.hpp"
#include <app/ParamWidget.hpp>

// Modified light widget for the white LED

struct WhiteLight : ModuleLightWidget {
    WhiteLight() {
        addBaseColor(SCHEME_WHITE);
    }
};

// Adapted light object for the glowing triangle

struct RGBTriangle : ModuleLightWidget {
    RGBTriangle() {
        addBaseColor(nvgRGBAf(1.0, 0.0, 0.0, 1.0));
        addBaseColor(nvgRGBAf(0.0, 1.0, 0.0, 1.0));
        addBaseColor(nvgRGBAf(0.0, 0.0, 1.0, 1.0));
    }
    
    void drawLight(const DrawArgs &args) override {
        
        nvgBeginPath(args.vg);
        nvgMoveTo(args.vg, .5,-17.8);
        nvgLineTo(args.vg, -12,9.6);
        nvgLineTo(args.vg, 12.7,9.6);
        nvgClosePath(args.vg);
        
        
        
        // Solid color
        
        nvgFillColor(args.vg, color);
        nvgTransRGBAf(color, 1.0);
        nvgFill(args.vg);
        
        // Border
        nvgStrokeWidth(args.vg, 0.5);
        nvgStrokeColor(args.vg, borderColor);
        nvgStroke(args.vg);
        nvgRotate(args.vg, (30.0/120.0)*NVG_PI*2);
    }
    
    void drawHalo(const DrawArgs &args) override {
        float radius = 14;
        float oradius = radius + 13;
        
        nvgBeginPath(args.vg);
        nvgRect(args.vg, -25, -25, 50, 50);
        
        NVGpaint paint;
        NVGcolor icol = color::mult(color, 0.10);
        NVGcolor ocol = nvgRGB(0, 0, 0);
        paint = nvgRadialGradient(args.vg, 0, 0, radius, oradius, icol, ocol);
        nvgFillPaint(args.vg, paint);
        nvgGlobalCompositeOperation(args.vg, NVG_LIGHTER);
        nvgFill(args.vg);
    }
};

struct ViaKnob : RoundKnob {

    void onButton(const event::Button &e) override {
        OpaqueWidget::onButton(e);

        // Touch parameter
        if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT && (e.mods & RACK_MOD_MASK) == 0) {
            if (paramQuantity) {
                APP->scene->rack->touchedParam = this;
            }
            e.consume(this);
        }

        // Right click to open context menu
        if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT && (e.mods & RACK_MOD_MASK) == 0) {
            createViaContextMenu();
            e.consume(this);
        }
    }

    struct ViaParamLabel : ui::MenuLabel {
        ParamWidget *paramWidget;
        void step() override {
            text = paramWidget->paramQuantity->getString();
            MenuLabel::step();
        }
    };


    struct ViaParamResetItem : ui::MenuItem {
        ParamWidget *paramWidget;
        void onAction(const event::Action &e) override {
            paramWidget->resetAction();
        }
    };


    struct ViaParamFineItem : ui::MenuItem {
    };


    struct ViaParamUnmapItem : ui::MenuItem {
        ParamWidget *paramWidget;
        void onAction(const event::Action &e) override {
            engine::ParamHandle *paramHandle = APP->engine->getParamHandle(paramWidget->paramQuantity->module->id, paramWidget->paramQuantity->paramId);
            if (paramHandle) {
                APP->engine->updateParamHandle(paramHandle, -1, 0);
            }
        }
    };

    void createViaContextMenu(void) {

        ui::Menu *menu = createMenu();

        ViaParamLabel *paramLabel = new ViaParamLabel;
        paramLabel->paramWidget = this;
        menu->addChild(paramLabel);

        // ParamField *paramField = new ParamField;
        // paramField->box.size.x = 100;
        // paramField->setParamWidget(this);
        // menu->addChild(paramField);

        ViaParamResetItem *resetItem = new ViaParamResetItem;
        resetItem->text = "Initialize";
        resetItem->rightText = "Double-click";
        resetItem->paramWidget = this;
        menu->addChild(resetItem);

        // ViaParamFineItem *fineItem = new ViaParamFineItem;
        // fineItem->text = "Fine adjust";
        // fineItem->rightText = RACK_MOD_CTRL_NAME "+drag";
        // fineItem->disabled = true;
        // menu->addChild(fineItem);

        engine::ParamHandle *paramHandle = paramQuantity ? APP->engine->getParamHandle(paramQuantity->module->id, paramQuantity->paramId) : NULL;
        if (paramHandle) {
            ViaParamUnmapItem *unmapItem = new ViaParamUnmapItem;
            unmapItem->text = "Unmap";
            unmapItem->rightText = paramHandle->text;
            unmapItem->paramWidget = this;
            menu->addChild(unmapItem);
        }

    }

};

struct ViaSifamBlack : ViaKnob {
    ViaSifamBlack() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/knob_sifam_blkcap.svg")));
    }
};

struct ViaSifamGrey : ViaKnob {
    ViaSifamGrey() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/knob_sifam_grycap.svg")));
    }
};

struct ViaJack : SvgPort {
    ViaJack() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/jack-nogradients.svg")));
    }
};

// Button skins for the manual trigger and touch sensors

struct ViaSwitch : SvgSwitch {

    void onButton(const event::Button &e) override {
        OpaqueWidget::onButton(e);

        // Touch parameter
        if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT && (e.mods & RACK_MOD_MASK) == 0) {
            if (paramQuantity) {
                APP->scene->rack->touchedParam = this;
            }
            e.consume(this);
        }

        // Right click to open context menu
        if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT && (e.mods & RACK_MOD_MASK) == 0) {
            createViaContextMenu();
            e.consume(this);
        }
    }

    struct ViaParamLabel : ui::MenuLabel {
        ParamWidget *paramWidget;
        void step() override {
            text = paramWidget->paramQuantity->getString();
            MenuLabel::step();
        }
    };


    struct ViaParamResetItem : ui::MenuItem {
        ParamWidget *paramWidget;
        void onAction(const event::Action &e) override {
            paramWidget->resetAction();
        }
    };


    struct ViaParamFineItem : ui::MenuItem {
    };


    struct ViaParamUnmapItem : ui::MenuItem {
        ParamWidget *paramWidget;
        void onAction(const event::Action &e) override {
            engine::ParamHandle *paramHandle = APP->engine->getParamHandle(paramWidget->paramQuantity->module->id, paramWidget->paramQuantity->paramId);
            if (paramHandle) {
                APP->engine->updateParamHandle(paramHandle, -1, 0);
            }
        }
    };

    void createViaContextMenu(void) {

        ui::Menu *menu = createMenu();

        ViaParamLabel *paramLabel = new ViaParamLabel;
        paramLabel->paramWidget = this;
        menu->addChild(paramLabel);

        // ParamField *paramField = new ParamField;
        // paramField->box.size.x = 100;
        // paramField->setParamWidget(this);
        // menu->addChild(paramField);

        ViaParamResetItem *resetItem = new ViaParamResetItem;
        resetItem->text = "Initialize";
        resetItem->rightText = "Double-click";
        resetItem->paramWidget = this;
        menu->addChild(resetItem);

        // ViaParamFineItem *fineItem = new ViaParamFineItem;
        // fineItem->text = "Fine adjust";
        // fineItem->rightText = RACK_MOD_CTRL_NAME "+drag";
        // fineItem->disabled = true;
        // menu->addChild(fineItem);

        engine::ParamHandle *paramHandle = paramQuantity ? APP->engine->getParamHandle(paramQuantity->module->id, paramQuantity->paramId) : NULL;
        if (paramHandle) {
            ViaParamUnmapItem *unmapItem = new ViaParamUnmapItem;
            unmapItem->text = "Unmap";
            unmapItem->rightText = paramHandle->text;
            unmapItem->paramWidget = this;
            menu->addChild(unmapItem);
        }

    }

};

struct TransparentButton : ViaSwitch {

    TransparentButton() {
        momentary = true;
        shadow->opacity = 0.0;
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/transparent_button.svg")));
    }
};

struct ViaPushButton : ViaSwitch {
    ViaPushButton() {
        momentary = true;
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/manual_trig.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/manual_trig_down.svg")));
    }
};
