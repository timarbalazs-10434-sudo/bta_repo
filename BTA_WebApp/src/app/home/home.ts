import { Component, inject, OnInit } from '@angular/core';
import { ActivatedRoute } from '@angular/router';
import { InventoryService } from '../services/inventory-service';

@Component({
  selector: 'app-home',
  imports: [],
  templateUrl: './home.html',
  styleUrl: './home.css',
})
export class Home {

  sajttortaRoute= "https://www.msn.com/hu-hu/hirek/other/" + 
  "a-t%C3%B6k%C3%A9letes-citromos-sajttorta-receptje-kr%C3%"+
  "A9mes-k%C3%B6nny%C5%B1-isteni/ar-AA1XHRhI?ocid=msedgntp&pc"+
  "=U531&cvid=69c0eceaca7f49d791c8f8e9cccf1b9b&ei=66";
  sajttortaOpen(){
    if (typeof window !== 'undefined') {
    window.open(this.sajttortaRoute, '_self');
    }
  }

  asd(){
    if (typeof window !== 'undefined') {
      window.open(this.sajttortaRoute, '_self');
    }
  }
}

